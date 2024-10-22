#pragma once

#include "wecs/signal/sigh.hpp"
#include "wecs/entity/storage.hpp"

namespace wecs {

// Type: BasicStorage
template <typename Type>
class SighMixin final : public Type {
public:
    using underlying_type = Type;
    using entity_type = typename underlying_type::entity_type;
    using payload_type = typename underlying_type::payload_type;
    using sigh_type = Sigh<void(entity_type, payload_type&)>;

    SighMixin() : underlying_type{} {}

    template <typename... Args>
    auto& emplace(entity_type entity, Args&&... args) {
        auto& payload = underlying_type::emplace(entity, std::forward<Args>(args)...);
        construction_.trigger(entity, payload);
        return payload;
    }

    template <typename... Func>
    auto& patch(entity_type entity, Func&&... func) {
        auto& payload = underlying_type::patch(entity, std::forward<Func>(func)...);
        update_.trigger(entity, payload);
        return payload;
    }

    void remove(entity_type entity) {
        auto& payload = underlying_type::operator[](entity);
        destruction_.trigger(entity, payload);
        underlying_type::remove(entity);
    }

    void clear() {
        underlying_type::clear();
    }

public:
    auto on_construct() noexcept {
        return Sink{construction_};
    }

    auto on_update() noexcept {
        return Sink{update_};
    }

    auto on_destruction() noexcept {
        return Sink{destruction_};
    }

private:
    sigh_type construction_;
    sigh_type destruction_;
    sigh_type update_;
};

template <typename EntityType, size_t PageSize>
class SighMixin<BasicStorage<EntityType, EntityType, PageSize, void>>
    : public BasicStorage<EntityType, EntityType, PageSize, void> {
public:
    using underlying_type = BasicStorage<EntityType, EntityType, PageSize, void>;
    using entity_type = typename underlying_type::entity_type;
    using sigh_type = Sigh<void(entity_type)>;

    auto emplace() {
        auto entity = underlying_type::emplace();
        construction_.trigger(entity);
        return entity;
    }

    void remove(entity_type entity) {
        destruction_.trigger(entity);
        underlying_type::remove(entity);
    }

    void clear() {
        underlying_type::clear();
    }

public:
    auto on_construct() noexcept {
        return Sink{construction_};
    }

    auto on_destruction() noexcept {
        return Sink{destruction_};
    }

private:
    sigh_type construction_;
    sigh_type destruction_;
};

} // namespace wecs