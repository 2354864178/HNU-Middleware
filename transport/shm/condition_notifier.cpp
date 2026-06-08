#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <thread>

#include "condition_notifier.h"
#include "../../common/util.h"
#include "../../common/log.h"

namespace hnu {
namespace Middleware {
namespace transport {

using common::Hash;

ConditionNotifier::ConditionNotifier() {
    key_ = static_cast<key_t>(Hash("/HNU-Middleware/transport/shm/notifier")); // 生成一个唯一的key，作为共享内存的标识符
    shm_size_ = sizeof(Indicator);                                             // 共享内存的大小，等于Indicator结构体的大小，因为只在共享内存中存储一个Indicator对象

    // 初始化共享内存，如果失败则输出错误日志并将is_shutdown_设置为true，表示通知器已经关闭，无法使用
    if (!Init()) {
        AERROR << "fail to init condition notifier.";
        is_shutdown_.store(true);
        return;
    }

    next_seq_ = indicator_->next_seq.load(); // 从共享内存中读取当前的next_seq值，表示下一个可用的序列号，初始值为0
    ADEBUG << "next_seq: " << next_seq_;
}

ConditionNotifier::~ConditionNotifier() {
    Shutdown();
}

void ConditionNotifier::Shutdown() {
    // 使用exchange方法将is_shutdown_设置为true，并返回之前的值
    // 如果之前已经是true，说明通知器已经关闭，直接返回
    if (is_shutdown_.exchange(true)) {
        return;
    }

    // 等待一段时间，确保正在执行Notify或Listen的线程能够感知到is_shutdown_的变化
    // 避免在它们还在访问共享内存时就删除共享内存，导致访问非法内存
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Reset(); // 断开与共享内存的连接，释放资源
}

// Notify方法用于通知可读信息，参数info表示要通知的信息，返回值表示是否成功
bool ConditionNotifier::Notify(const ReadableInfo& info) {
    if (is_shutdown_.load()) {
        ADEBUG << "notifier is shutdown.";
        return false;
    }

    uint64_t seq = indicator_->next_seq.fetch_add(1); // 获取当前的next_seq值，然后next_seq自增1

    // 填充要通知的信息
    uint64_t idx = seq % kBufLength; // 要写入的位置
    indicator_->infos[idx] = info;   // 将info写入共享内存中对应位置的infos数组
    indicator_->seqs[idx] = seq;     // 将seq写入共享内存中对应位置的seqs数组，表示这个位置已经被写入了一个新的信息
    // next_seq比indicator_->seqs[idx]大1，这个信息在Listen方法中很重要
    return true;
}

// Listen方法用于监听可读信息，参数timeout_ms表示等待通知的超时时间(ms)，参数info用于存储接收到的可读信息
bool ConditionNotifier::Listen(int timeout_ms, ReadableInfo* info) {
    if (info == nullptr) {
        AERROR << "info nullptr";
        return false;
    }

    if (is_shutdown_.load()) {
        ADEBUG << "notifier is shutdown.";
        return false;
    }

    int timeout_us = timeout_ms * 1000;
    while (!is_shutdown_.load()) {

        uint64_t seq = indicator_->next_seq.load();

        // 如果有其他进程 执行了Notify，则 seq > next_seq_ ,说明有新的info
        // 这里内层为什么加等号有点绕
        /*
        读者期望 next_seq_ = 4
        写者已经把全局 indicator_->next_seq 推到 5
        读者进外层，seq = 5，next_seq_ = 4，所以能进去
        读者去看槽位 idx = 4 % kBufLength
        这个槽位里 actual_seq = 4
        于是内层判断成立，说明这条数据已经写好，可以读了，读者把 next_seq_ 推到 5
        */
        if (seq != next_seq_) {
            auto idx = next_seq_ % kBufLength;
            auto actual_seq = indicator_->seqs[idx];

            if (actual_seq >= next_seq_) {
                next_seq_ = actual_seq;
                *info = indicator_->infos[idx];
                ++next_seq_;
                return true;
            } else {
                ADEBUG << "seq[" << next_seq_ << "] is writing, can not read now.";
            }
        }

        if (timeout_us > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(50));
            timeout_us -= 50;
        } else {
            return false;
        }
    }

    ADEBUG << "debug listen";
    return false;
}

bool ConditionNotifier::Init() {
    return OpenOrCreate();
}

// 创建共享内存，并将Indicator对象放在共享内存中
bool ConditionNotifier::OpenOrCreate() {
    int retry = 0; // 最大重试次数，避免死循环
    int shmid = 0; // 共享内存ID，shmget函数的返回值，如果成功则是一个非负整数，表示共享内存的标识符，如果失败则是-1
    while (retry < 2) {
        shmid = shmget(key_, shm_size_, 0644 | IPC_CREAT | IPC_EXCL);
        if (shmid != -1) {
            break;
        }

        // 如果共享内存已经存在但是大小不匹配，删除旧的共享内存并重新创建一个新的共享内存
        if (EINVAL == errno) {
            AINFO << "need larger space, recreate.";
            Reset();
            Remove();
            ++retry;
        }

        // 如果共享内存已经存在且大小匹配，直接打开这个共享内存并映射到当前进程的地址空间
        else if (EEXIST == errno) {
            ADEBUG << "shm already exist, open only.";
            return OpenOnly();
        }

        else
            break;
    }

    // 如果创建共享内存失败，输出错误日志并返回false
    if (shmid == -1) {
        AERROR << "create shm failed, error code: " << strerror(errno);
        return false;
    }

    managed_shm_ = shmat(shmid, nullptr, 0); // 把共享内存映射到当前进程的地址空间

    // 如果映射失败，输出错误日志，删除共享内存，并返回false
    if (managed_shm_ == reinterpret_cast<void*>(-1)) {
        AERROR << "attach shm failed.";
        shmctl(shmid, IPC_RMID, 0);
        return false;
    }

    indicator_ = new (managed_shm_) Indicator(); // 在共享内存中创建一个Indicator对象

    // 如果创建Indicator对象失败
    if (indicator_ == nullptr) {
        AERROR << "create indicator failed.";
        shmdt(managed_shm_);        // 断开与共享内存的连接
        managed_shm_ = nullptr;     // 避免悬空指针
        shmctl(shmid, IPC_RMID, 0); // 删除共享内存
        return false;
    }

    ADEBUG << "open or create true.";
    return true;
}

bool ConditionNotifier::OpenOnly() {
    int shmid = shmget(key_, 0, 0644);
    if (shmid == -1) {
        AERROR << "get shm failed, error: " << strerror(errno);
        return false;
    }

    // 映射共享内存
    managed_shm_ = shmat(shmid, nullptr, 0);
    if (managed_shm_ == reinterpret_cast<void*>(-1)) {
        AERROR << "attach shm failed.";
        shmctl(shmid, IPC_RMID, 0);
        return false;
    }

    // 把地址转成 Indicator*，直接当对象用，注意这里没有调用构造函数，因为之前已经创建过了
    indicator_ = reinterpret_cast<Indicator*>(managed_shm_);
    if (indicator_ == nullptr) {
        // 只是 attach 失败，不代表共享内存本身有问题，所以不删除共享内存
        AERROR << "get indicator failed.";
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
        return false;
    }

    ADEBUG << "Open true";

    return true;
}

// 删除共享内存
bool ConditionNotifier::Remove() {
    int shmid = shmget(key_, 0, 0644);
    if (shmid == -1 || shmctl(shmid, IPC_RMID, 0) == -1) {
        AERROR << "remove shm failed, error code: " << strerror(errno);
        return false;
    }

    ADEBUG << "remove success.";
    return true;
}

// shm detach 断开与共享内存的连接
void ConditionNotifier::Reset() {
    indicator_ = nullptr;
    if (managed_shm_ != nullptr) {
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
    }
}

} // namespace transport
} // namespace Middleware
} // namespace hnu