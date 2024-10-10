#pragma once

#include "wecs/signal/delegate.hpp"

namespace wecs {

class Connection {
    template <typename>
    friend class Sink;

    Connection(Delegate<void(void*)> fn, void* ref)
        : disconnect_{fn}, signal_{ref} {}

public:
    Connection() : disconnect_{}, signal_{} {}

    explicit operator bool() const noexcept {
        return static_cast<bool>(disconnect_);
    }

    void release() {
        if (disconnect_) {
            disconnect_(signal_);
            disconnect_.reset();
        }
    }

private:
    Delegate<void(void*)> disconnect_;
    void* signal_;
};

template <typename Sigh>
class Sink {
public:
    using sigh_type = Sigh;
    using delegate_type = typename sigh_type::delegate_type;
    using difference_type = typename sigh_type::container_type::difference_type;

    Sink(sigh_type& sigh) : sigh_{&sigh} {}

    template<auto Function, typename... Payload>
    Connection connect(Payload&&... args) {
        disconnect<Function>(args...);

        delegate_type delegate{};
        delegate.template connect<Function>(args...);
        sigh_->insert(std::move(delegate));

        Delegate<void(void*)> coon{};
        coon.connect<&release<Function, Payload...>>(args...);

        return {coon, sigh_};
    }

    template <auto Function, typename... Payload>
    void disconnect(Payload&&... args) {
        delegate_type delagate{};
        delagate.template connect<Function>(args...);
        disconnect_if([&delagate](const auto& elem) {
            return elem == delagate;
        });
    }

    void disconnect(const void* payload) {
        WECS_ASSERT(payload != nullptr, "invalid payload");
        disconnect_if([payload](const auto& elem) {
            return elem.data() == payload;
        });
    }

    void disconnect() {
        clear();
    }

public:
    bool empty() const noexcept {
        return sigh_->empty();
    }

    void clear() noexcept {
        sigh_->clear();
    }

private:
    template <typename Function>
    void disconnect_if(Function callback) {
        for (auto pos = sigh_->delegates_.size(); pos; --pos) {
            if (auto& elem = sigh_->delegates_[pos - 1u]; callback(elem)) {
                elem = std::move(sigh_->delegates_.back());
                sigh_->delegates_.pop_back();
            }
        }
    }

    template <auto Function, typename Payload>
    static void release(Payload payload, void* sigh) {
        Sink{*static_cast<sigh_type*>(sigh)}.disconnect<Function>(payload);
    }

    template<auto Function>
    static void release(void* sigh) {
        Sink{*static_cast<sigh_type*>(sigh)}.disconnect<Function>();
    }

private:
    sigh_type* sigh_;
};

} // namespace wecs