#pragma once

#include "wecs/core/ident.hpp"
#include "wecs/entity/mixin.hpp"
#include "wecs/entity/view.hpp"
#include <optional>

namespace wecs {

namespace internal {

template <typename SparseSet, typename Type>
struct StorageFor;

template <typename EntityType, size_t PageSize, typename Type>
struct StorageFor<BasicSparseSet<EntityType, PageSize>, Type> {
    using type = SighMixin<BasicStorage<EntityType, Type, PageSize, std::allocator<Type>>>;
};

template <typename SparseSet, typename Type>
using storage_for_t = typename StorageFor<SparseSet, Type>::type;

} // namespace internal

template <typename EntityType, size_t PageSize>
class BasicRegistry {
public:
    using base_type = BasicSparseSet<EntityType, PageSize>;
    using pool_container_type = std::vector<std::shared_ptr<base_type>>;
    template <typename Type>
    using storage_for_t = internal::storage_for_t<base_type, Type>;
    using entities_container_type = SighMixin<BasicStorage<EntityType, EntityType, PageSize, void>>;
    using component_ident = Ident<struct ComponentIdent>;

    template <typename... Types>
    using view_type = View<EntityType, BasicRegistry, Types...>;

    auto create() {
        auto entity = entities_.emplace();
        return entity;
    }

    void destroy(EntityType entity) {
        if (alive(entity)) {
            entities_.remove(entity);
            for (auto& pool : pools_) {
                if (pool->contain(entity)) {
                    pool->remove(entity);
                }
            }
        }
    }

    void clear() {
        entities_.clear();
        for (auto& pool : pools_) {
            pool->clear();
        }
    }

    bool alive(EntityType entity) const {
        return entities_.contain(entity);
    }

    template <typename Type, typename... Args>
    Type& emplace(EntityType entity, Args&&... args) {
        return assure<Type>().emplace(entity, std::forward<Args>(args)...);
    }

    template <typename Type, typename... Func>
    Type& patch(EntityType entity, Func&&... func) {
        return assure<Type>().patch(entity, std::forward<Func>(func)...);
    }

    template <typename Type, typename... Args>
    Type& replace(EntityType entity, Args&&... args) {
        return patch<Type>(entity, [&args...](auto&... component) {
            ((component = Type{std::forward<Args>(args)...}), ...);
        });
    }

    template <typename Type>
    bool has(EntityType entity) const {
        auto idx = component_ident::get<Type>();
        if (idx >= pools_.size()) {
            return false;
        }
        return pools_[idx] && pools_[idx]->contain(entity);
    }

    template <typename Type>
    void remove(EntityType entity) {
        assure<Type>().remove(entity);
    }

    template <typename Type>
    const Type& get(EntityType entity) const {
        auto idx = component_ident::get<Type>();
        return static_cast<storage_for_t<Type>&>(*pools_[idx])[entity];
    }

    template <typename Type>
    Type& get_mutable(EntityType entity) {
        return const_cast<Type&>(std::as_const(*this).template get<Type>(entity));
    }

    template <typename... Types>
    view_type<Types...> view() noexcept {
        WECS_ASSERT(sizeof...(Types) > 0, "you must provide query component");
        using view_list = typename view_type<Types...>::view_list;
        typename base_type::packed_container_type entities;
        auto min_idx = idx_of_min_num(component_idx(view_list{}));
        if (!min_idx.has_value()) {
            return view_type<Types...>(storages(view_list{}), {});
        }
        for (size_t i = 0; i < pools_[min_idx.value()]->size(); i++) {
            auto entity = pools_[min_idx.value()]->packed()[i];
            if (entity_has_other_component(static_cast<EntityType>(entity), view_list{})) {
                entities.push_back(entity);
            }
        }
        return view_type<Types...>(storages(view_list{}), entities);
    }

public:
    template <typename Type>
    auto on_construct() noexcept {
        if constexpr (std::is_same_v<Type, EntityType>) {
            return Sink{entities_.on_construct()};
        } else {
            return Sink{assure<Type>().on_construct()};
        }
    }

    template <typename Type>
    auto on_update() noexcept {
        return Sink{assure<Type>().on_update()};
    }

    template <typename Type>
    auto on_destruction() noexcept {
        if constexpr (std::is_same_v<Type, EntityType>) {
            return Sink{entities_.on_destruction()};
        } else {
            return Sink{assure<Type>().on_destruction()};
        }
    }

public:
    size_t size() const noexcept {
        return entities_.size();
    }

private:
    template <typename Type>
    storage_for_t<Type>& assure() {
        using storage_type = storage_for_t<Type>;
        auto idx = component_ident::get<Type>();
        if (idx >= pools_.size()) {
            pools_.resize(idx + 1);
        }
        if (!pools_[idx]) {
            pools_[idx] = std::make_shared<storage_type>();
        }
        return static_cast<storage_type&>(*pools_[idx]);
    }

    template <typename... Types>
    std::array<size_t, sizeof...(Types)> component_idx(TypeList<Types...>) const {
        return {component_ident::get<Types>()...};
    }

    template <size_t N>
    std::optional<size_t> idx_of_min_num(const std::array<size_t, N>& indices) {
        size_t min_num = std::numeric_limits<size_t>::max();
        size_t min_idx = 0;
        for (auto idx : indices) {
            if (idx >= pools_.size() || !pools_[idx]) {
                return std::nullopt;
            }
            if (min_num > pools_[idx]->size()) {
                min_num = pools_[idx]->size();
                min_idx = idx;
            }
        }
        return min_num == std::numeric_limits<size_t>::max() ? std::nullopt : std::make_optional(min_idx);
    }

    template <typename... Types>
    auto storages(TypeList<Types...>) {
        return std::tuple{&assure<Types>()...};
    }

    struct Check {
        template <typename T>
        bool has(EntityType entity, const pool_container_type& pools) const {
            auto idx = component_ident::get<T>();
            return idx < pools.size() && pools[idx] && pools[idx]->contain(entity);
        }
    };

    template <typename... Types>
    bool entity_has_other_component(EntityType entity, TypeList<Types...> list) {
        return check(entity, list, Check{});
    }

    template <typename T, typename... Ts, typename F>
    bool check(EntityType entity, TypeList<T, Ts...>, F f) {
        if constexpr (sizeof...(Ts) == 0) {
            return f.template has<T>(entity, pools_);
        } else {
            return f.template has<T>(entity, pools_) && check(entity, TypeList<Ts...>{}, f);
        }
    }

private:
    pool_container_type pools_;
    entities_container_type entities_;
};

} // namespace wecs