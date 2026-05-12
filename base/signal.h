#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <mutex>

namespace hnu {
namespace Middleware {
namespace base {

template <typename... Args> class Slot;

template <typename... Args> class Connection;

// 被观察者，表示一个信号，可以连接多个槽函数，当信号被触发时，所有连接的槽函数都会被调用
template <typename... Args> class Signal {
    public:
    using Callback = std::function<void(Args...)>;
    using SlotPtr = std::shared_ptr<Slot<Args...>>;
    using SlotList = std::list<SlotPtr>; // 槽函数列表，存储所有连接的槽函数对象，使用智能指针管理槽函数对象的生命周期
    using ConnectionType = Connection<Args...>;

    Signal() {}
    virtual ~Signal() {
        DisconnectAllSlots();
    }

    // 触发信号，调用所有连接的槽函数，传递参数给槽函数，线程安全
    void operator()(Args... args) {
        SlotList local;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            // 将槽函数列表复制到一个局部变量中，避免在调用槽函数时持有锁，减少锁的粒度，提高性能
            for (auto& slot : slots_) {
                local.emplace_back(slot);
            }
        }

        if (!local.empty()) {
            for (auto& slot : local) {
                (*slot)(args...);
            }
        }

        ClearDisconnectedSlots();
    }

    // 连接一个槽函数，接受一个回调函数参数，创建一个槽函数对象并添加到槽函数列表中，返回一个连接对象用于管理连接关系
    ConnectionType Connect(const Callback& cb) {
        auto slot = std::make_shared<Slot<Args...>>(cb);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            slots_.emplace_back(slot);
        }

        return ConnectionType(slot, this);
    }

    bool Disconnect(const ConnectionType& conn) {
        bool find = false;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& slot : slots_) {
                if (conn.HasSlot(slot)) {
                    find = true;
                    slot->Disconnect();
                }
            }
        }

        if (find) {
            ClearDisconnectedSlots();
        }
        return find;
    }

    void DisconnectAllSlots() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& slot : slots_) {
            slot->Disconnect();
        }
        slots_.clear();
    }

    private:
    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    void ClearDisconnectedSlots() {
        std::lock_guard<std::mutex> lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(), [](const SlotPtr& slot) { return !slot->connected(); }), slots_.end());
    }

    SlotList slots_;   // 槽函数列表，存储所有连接的槽函数对象，使用智能指针管理槽函数对象的生命周期
    std::mutex mutex_; // 互斥锁，保护槽函数列表的访问，确保线程安全
};

// 连接类，表示一个连接关系，包含一个槽函数指针和一个信号指针，可以通过连接对象来判断连接状态和断开连接
template <typename... Args> class Connection {
    public:
    using SlotPtr = std::shared_ptr<Slot<Args...>>;
    using SignalPtr = Signal<Args...>*;

    Connection() : slot_(nullptr), signal_(nullptr) {}
    Connection(const SlotPtr& slot, const SignalPtr& signal) : slot_(slot), signal_(signal) {}
    virtual ~Connection() {
        slot_ = nullptr;
        signal_ = nullptr;
    }

    Connection& operator=(const Connection& another) {
        if (this != &another) {
            this->slot_ = another.slot_;
            this->signal_ = another.signal_;
        }
        return *this;
    }

    bool HasSlot(const SlotPtr& slot) const {
        if (slot != nullptr && slot_ != nullptr) {
            return slot_.get() == slot.get();
        }
        return false;
    }

    bool IsConnected() const {
        if (slot_) {
            return slot_->connected();
        }
        return false;
    }

    bool Disconnect() {
        if (signal_ && slot_) {
            return signal_->Disconnect(*this);
        }
        return false;
    }

    private:
    SlotPtr slot_;
    SignalPtr signal_;
};

// 观察者，表示一个槽函数，可以连接到一个信号，当信号被触发时，槽函数会被调用，可以通过槽函数对象来判断连接状态和断开连接
template <typename... Args> class Slot {
    public:
    using Callback = std::function<void(Args...)>;
    Slot(const Slot& another) : cb_(another.cb_), connected_(another.connected_) {}
    explicit Slot(const Callback& cb, bool connected = true) : cb_(cb), connected_(connected) {}
    virtual ~Slot() {}

    void operator()(Args... args) {
        if (connected_ && cb_) {
            cb_(args...);
        }
    }

    void Disconnect() {
        connected_ = false;
    }
    bool connected() const {
        return connected_;
    }

    private:
    Callback cb_;
    bool connected_ = true;
};
} // namespace base
} // namespace Middleware
} // namespace hnu
