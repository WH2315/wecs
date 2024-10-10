#pragma once

#include "wecs/signal/delegate.hpp"

namespace wecs {

template <typename T>
class Sink;

template <typename T>
class Sigh;

template <typename Ret, typename... Args>
class Sigh<Ret(Args...)> {
    friend class Sink<Sigh>;

public:
    using delegate_type = Delegate<Ret(Args...)>;
    using container_type = std::vector<delegate_type>;

    void insert(const delegate_type& delegate) {
        delegates_.push_back(delegate);
    }

    void remove(delegate_type delegate) {
        auto it = std::remove(delegates_.begin(), delegates_.end(), delegate);
        delegates_.erase(it, delegates_.end());
    }

    void trigger(Args... args) {
        for (auto pos = delegates_.size(); pos; --pos) {
            delegates_[pos - 1](args...);
        }
    }

public:
    size_t size() const noexcept {
        return delegates_.size();
    }

    bool empty() const noexcept {
        return delegates_.empty();
    }

    void clear() noexcept {
        delegates_.clear();
    }

private:
    container_type delegates_;
};

} // namespace wecs