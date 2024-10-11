#pragma once

#include "wecs/signal/sigh.hpp"
#include "wecs/signal/sink.hpp"

namespace wecs {

template <typename Type, typename... Args>
class Dispatcher {
public:
    using sigh_type = Sigh<void(const Type&, Args...)>;
    using container_type = std::vector<Type>;

    auto sink() {
        return Sink{sigh_};
    }

    template <typename... Ts>
    void enqueue(Ts&&... ts) {
        events_.emplace_back(std::forward<Ts>(ts)...);
    }

    void trigger(const Type& event, Args... args) {
        sigh_.trigger(event, std::forward<Args>(args)...);
    }

    void update(Args... args) {
        while (!events_.empty()) {
            trigger(events_[0], std::forward<Args>(args)...);
            std::swap(events_[0], events_.back());
            events_.pop_back();
        }
    }

public:
    size_t size() const noexcept {
        return sigh_.size();
    }

    bool empty() const noexcept {
        return sigh_.empty();
    }

    void clear() noexcept {
        sigh_.clear();
    }

private:
    sigh_type sigh_;
    container_type events_;
};

} // namespace wecs