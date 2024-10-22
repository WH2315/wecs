#pragma once

#include "wecs/core/ident.hpp"
#include "wecs/signal/sigh.hpp"
#include "wecs/signal/sink.hpp"
#include <memory>

namespace wecs {

struct BasicDispatcherHandler {
    virtual ~BasicDispatcherHandler() = default;
    virtual void update() = 0;
    virtual void clear() = 0;
};

template <typename Type>
class DispatcherHandler : public BasicDispatcherHandler {
public:
    static_assert(std::is_same_v<Type, std::decay_t<Type>>, "Invalid type");

    using sigh_type = Sigh<void(const Type&)>;
    using container_type = std::vector<Type>;

    auto sink() { return Sink{sigh_}; }

    template <typename... Args>
    void enqueue(Args&&... args) {
        if constexpr(std::is_aggregate_v<Type> && (sizeof...(Args) != 0u || !std::is_default_constructible_v<Type>)) {
            events_.push_back(Type{std::forward<Args>(args)...});
        } else {
            events_.emplace_back(std::forward<Args>(args)...);
        }
    }

    void trigger(const Type& event) {
        sigh_.trigger(event);
    }

    void update() override {
        while (!events_.empty()) {
            trigger(events_[0]);
            std::swap(events_[0], events_.back());
            events_.pop_back();
        }
    }

    void clear() override {
        events_.clear();
    }

private:
    sigh_type sigh_;
    container_type events_;
};

class Dispatcher {
public:
    template <typename Type>
    using handler_type = DispatcherHandler<Type>;

    using container_type = std::vector<std::unique_ptr<BasicDispatcherHandler>>;
    using dispatcher_ident = Ident<struct DispatcherIdent>;

    template <typename Type>
    auto sink() {
        return assure<Type>().sink();
    }

    template <typename Type, typename... Args>
    void enqueue(Args&&... args) {
        assure<Type>().enqueue(std::forward<Args>(args)...);
    }

    template <typename Type>
    void enqueue(Type&& event) {
        assure<std::decay_t<Type>>().enqueue(std::forward<Type>(event));
    }

    template <typename Type, typename... Args>
    void trigger(Args&&... args) {
        assure<Type>().trigger(Type{std::forward<Args>(args)...});
    }

    template <typename Type>
    void trigger(Type&& event) {
        assure<std::decay_t<Type>>().trigger(std::forward<Type>(event));
    }

    template <typename Type>
    void update() {
        assure<Type>().update();
    }

    void update() const {
        for (auto pos = pools_.size(); pos; --pos) {
            if (auto&& pool = pools_[pos - 1]; pool) {
                pool->update();
            }
        }
    }

    template <typename... Type>
    void clear() {
        if constexpr (sizeof...(Type) == 0) {
            for (auto&& pool : pools_) {
                if (pool) {
                    pool->clear();
                }
            }
        } else {
            (assure<Type>().clear(), ...);
        }
    }

private:
    template <typename Type>
    handler_type<Type>& assure() {
        auto idx = dispatcher_ident::get<Type>();
        if (idx >= pools_.size()) {
            pools_.resize(idx + 1);
        }
        if (!pools_[idx]) {
            pools_[idx] = std::make_unique<handler_type<Type>>();
        }
        return static_cast<handler_type<Type>&>(*pools_[idx]);
    }

private:
    container_type pools_;
};

} // namespace wecs