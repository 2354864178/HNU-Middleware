
#include <sys/ipc.h>
#include <sys/shm.h>

#include "xsi_segment.h"
#include "common/log.h"

namespace hnu {
namespace Middleware {
namespace transport {

// XsiSegment的构造函数，接受一个channel_id参数，用于初始化channel_id_成员变量，同时将其他成员变量初始化为默认值
XsiSegment::XsiSegment(uint64_t channel_id) : Segment(channel_id) {
    key_ = static_cast<key_t>(channel_id);
}

// XsiSegment的析构函数，调用Destroy()方法销毁共享内存，释放资源
XsiSegment::~XsiSegment() {
    Destroy();
}

// 打开或创建共享内存，如果共享内存已经存在则打开它，否则创建一个新的共享内存段，并将其映射到当前进程的地址空间中，同时初始化State对象和Block数组，并为每个Block buf创建内存
bool XsiSegment::OpenOrCreate() {
    // 如果共享内存已经初始化成功，则直接返回true，表示可以使用这个共享内存段进行读写操作
    if (init_) {
        return true;
    }

    int retry = 0; // 记录尝试创建共享内存的次数，初始值为0，每次尝试创建失败后增加1，如果超过最大重试次数则放弃创建共享内存
    int shmid = 0; // 共享内存的标识符，shmget()函数成功创建或打开共享内存后返回一个非负整数作为共享内存的标识符，如果创建或打开失败则返回-1，此时可以通过errno来获取错误码，判断失败的原因
    // 创建共享内存
    while (retry < 2) {
        shmid = shmget(key_, conf_.managed_shm_size(), 0644 | IPC_CREAT | IPC_EXCL);
        if (shmid != -1) { // 创建共享内存成功，跳出循环
            break;
        }

        // errno为EINVAL表示创建共享内存失败，可能是因为消息大小超过上限导致共享内存配置不合适
        if (EINVAL == errno) {
            AINFO << "need larger space, recreate.";
            Reset();
            Remove();
            ++retry;
        }
        // errno为EEXIST表示共享内存已经存在，可能是因为之前的进程已经创建了共享内存段，但当前进程还没有映射它，此时可以尝试打开已存在的共享内存段
        else if (EEXIST == errno) {
            ADEBUG << "shm already exist, open only.";
            return OpenOnly();
        } else {
            break;
        }
    }

    if (shmid == -1) {
        AERROR << "create shm failed, error code: " << strerror(errno);
        return false;
    }

    // 映射共享内存
    managed_shm_ = shmat(shmid, nullptr, 0);
    // shmat()如果映射失败则返回(void*)-1，此时可以通过errno来获取错误码，判断失败的原因
    if (managed_shm_ == reinterpret_cast<void*>(-1)) {
        AERROR << "attach shm failed, error: " << strerror(errno);
        shmctl(shmid, IPC_RMID, 0); // 删除共享内存
        return false;
    }

    state_ = new (managed_shm_) State(conf_.ceiling_msg_size());
    if (state_ == nullptr) {
        AERROR << "create state failed.";
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
        shmctl(shmid, IPC_RMID, 0);
        return false;
    }

    conf_.Update(state_->ceiling_msg_size());

    blocks_ = new (static_cast<char*>(managed_shm_) + sizeof(State)) Block[conf_.block_num()];
    if (blocks_ == nullptr) {
        AERROR << "create blocks failed.";
        state_->~State();
        state_ = nullptr;
        shmdt(managed_shm_); // 解除映射
        managed_shm_ = nullptr;
        shmctl(shmid, IPC_RMID, 0);
        return false;
    }

    // 为每个 Block buf 创建内存
    uint32_t i = 0;
    for (; i < conf_.block_num(); ++i) {
        uint8_t* addr = new (static_cast<char*>(managed_shm_) + sizeof(State) + conf_.block_num() * sizeof(Block) + i * conf_.block_buf_size()) uint8_t[conf_.block_buf_size()];
        std::lock_guard<std::mutex> _g(block_buf_lock_);
        block_buf_addrs_[i] = addr;
    }

    if (i != conf_.block_num()) {
        AERROR << "create block buf failed.";
        state_->~State();
        state_ = nullptr;
        blocks_ = nullptr;
        {
            std::lock_guard<std::mutex> _g(block_buf_lock_);
            block_buf_addrs_.clear();
        }
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
        shmctl(shmid, IPC_RMID, 0);
        return false;
    }

    state_->IncreaseReferenceCounts();
    init_ = true;
    ADEBUG << "open or create true.";
    return true;
}

bool XsiSegment::OpenOnly() {
    if (init_) {
        return true;
    }

    int shmid = shmget(key_, 0, 0644);
    if (shmid == -1) {
        AERROR << "get shm failed. error: " << strerror(errno);
        return false;
    }
    // attach managed_shm_
    managed_shm_ = shmat(shmid, nullptr, 0);
    if (managed_shm_ == reinterpret_cast<void*>(-1)) {
        AERROR << "attach shm failed, error: " << strerror(errno);
        return false;
    }

    // get field state_
    state_ = reinterpret_cast<State*>(managed_shm_);
    if (state_ == nullptr) {
        AERROR << "get state failed.";
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
        return false;
    }

    conf_.Update(state_->ceiling_msg_size());

    // get field blocks_
    blocks_ = reinterpret_cast<Block*>(static_cast<char*>(managed_shm_) + sizeof(State));

    if (blocks_ == nullptr) {
        AERROR << "get blocks failed.";
        state_ = nullptr;
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
        return false;
    }
    // get block buf
    uint32_t i = 0;
    for (; i < conf_.block_num(); ++i) {
        uint8_t* addr = reinterpret_cast<uint8_t*>(static_cast<char*>(managed_shm_) + sizeof(State) + conf_.block_num() * sizeof(Block) + i * conf_.block_buf_size());

        if (addr == nullptr) {
            break;
        }
        std::lock_guard<std::mutex> _g(block_buf_lock_);
        block_buf_addrs_[i] = addr;
    }

    if (i != conf_.block_num()) {
        AERROR << "open only failed.";
        state_->~State();
        state_ = nullptr;
        blocks_ = nullptr;
        {
            std::lock_guard<std::mutex> _g(block_buf_lock_);
            block_buf_addrs_.clear();
        }
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
        shmctl(shmid, IPC_RMID, 0);
        return false;
    }

    state_->IncreaseReferenceCounts();
    init_ = true;
    ADEBUG << "open only true.";
    return true;
}

bool XsiSegment::Remove() {
    int shmid = shmget(key_, 0, 0644);
    if (shmid == -1 || shmctl(shmid, IPC_RMID, 0) == -1) {
        AERROR << "remove shm failed, error code: " << strerror(errno);
        return false;
    }
    ADEBUG << "remove success.";
    return true;
}

void XsiSegment::Reset() {
    state_ = nullptr;
    blocks_ = nullptr;
    {
        std::lock_guard<std::mutex> _g(block_buf_lock_);
        block_buf_addrs_.clear();
    }
    if (managed_shm_ != nullptr) {
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
        return;
    }
}

} // namespace transport
} // namespace Middleware
} // namespace hnu
