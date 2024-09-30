#pragma once

#include <type_traits>

namespace wecs {

namespace internal {

template <typename, typename = void>
struct EntityTraits;

template <typename EntityType>
struct EntityTraits<EntityType, std::enable_if_t<std::is_enum_v<EntityType>>>
    : EntityTraits<std::underlying_type_t<EntityType>> {
  using value_type = EntityType;
};

template <typename EntityType>
struct EntityTraits<EntityType, std::enable_if_t<std::is_class_v<EntityType>>>
    : EntityTraits<typename EntityType::entity_type> {
  using value_type = EntityType;
};

template <>
struct EntityTraits<uint32_t> {
    using value_type = uint32_t;
    using entity_type = uint32_t;
    using version_type = uint16_t;

    static constexpr uint32_t entity_mask = 0xfffff;
    static constexpr uint16_t version_mask = 0xfff;
};

} // namespace internal

template <typename T>
static constexpr int popcount(T x) noexcept {
    return x ? static_cast<int>((x & 1) + popcount(x >> 1)) : 0;
}

template <typename EntityTraits>
class BasicEntityTraits {
    static constexpr auto length = popcount(EntityTraits::entity_mask);

public:
    using value_type = typename EntityTraits::value_type;
    using entity_type = typename EntityTraits::entity_type;
    using version_type = typename EntityTraits::version_type;

    static constexpr entity_type entity_mask = EntityTraits::entity_mask;
    static constexpr version_type version_mask = EntityTraits::version_mask;

    static constexpr entity_type to_integral(const value_type value) {
        return static_cast<entity_type>(value);
    }

    static constexpr entity_type to_entity(const value_type value) {
        return to_integral(value) & entity_mask;
    }

    static constexpr version_type to_version(const value_type value) {
        return static_cast<version_type>((to_integral(value) >> length) & version_mask);
    }

    static constexpr value_type construct(const entity_type entity, const version_type version) {
        return value_type{((entity & entity_mask) | static_cast<entity_type>(version & version_mask) << length)};
    }

    //! @brief combine lhs's entity with rhs's version
    static constexpr value_type combine(const entity_type lhs, const entity_type rhs) {
        return value_type{(lhs & entity_mask) | (rhs & (version_mask << length))};
    }

    static constexpr value_type next(const value_type value) {
        const auto vers = to_version(value) + 1;
        return construct(to_integral(value), static_cast<version_type>(vers + (vers == version_mask)));
    }
};

template <typename EntityType>
struct EntityTraits : BasicEntityTraits<internal::EntityTraits<EntityType>> {
    using base_type = BasicEntityTraits<internal::EntityTraits<EntityType>>;
};

template <typename EntityType>
constexpr auto to_integral(const EntityType value) {
    return EntityTraits<EntityType>::to_integral(value);
}

template <typename EntityType>
constexpr auto to_entity(const EntityType value) {
    return EntityTraits<EntityType>::to_entity(value);
}

template <typename EntityType>
constexpr auto to_version(const EntityType value) {
    return EntityTraits<EntityType>::to_version(value);
}

struct EmptyEntity {
    template <typename EntityType>
    constexpr operator EntityType() const {
        using traits = EntityTraits<EntityType>;
        return traits::construct(traits::entity_mask, traits::version_mask);
    }

    constexpr bool operator==(EmptyEntity) const { return true; }
    constexpr bool operator!=(EmptyEntity) const { return false; }

    template <typename EntityType>
    constexpr bool operator==(const EntityType entity) const {
        using traits = EntityTraits<EntityType>;
        return traits::to_entity(entity) == traits::to_entity(*this);
    }

    template <typename EntityType>
    constexpr bool operator!=(const EntityType entity) const {
        return !(entity == *this);
    }
};

template <typename EntityType>
constexpr bool operator==(const EntityType lhs, const EmptyEntity rhs) noexcept {
    return rhs.operator==(lhs);
}

template <typename EntityType>
constexpr bool operator!=(const EntityType lhs, const EmptyEntity rhs) noexcept {
    return !(rhs == lhs);
}

inline constexpr EmptyEntity empty_entity = {};

} // namespace wecs