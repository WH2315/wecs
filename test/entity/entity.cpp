#include "wecs/wecs.hpp"
#include "wecs/entity/entity.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace wecs;

#define Entity(x) static_cast<entity>(x)

TEST_CASE("entity") {
    REQUIRE(popcount(0b11111) == 5);
    REQUIRE(popcount(0b11111111111111) == 14);
    REQUIRE(popcount(0x0) == 0);
    REQUIRE(popcount(0b001001010) == 3);

    auto entity_1 = Entity(3);
    auto entity_2 = Entity(0xffffffff);
    REQUIRE(to_integral(entity_1) == 3);
    REQUIRE(to_entity(entity_1) == 3);
    REQUIRE(to_version(entity_1) == 0);
    REQUIRE(to_integral(entity_2) == 0xffffffff);
    REQUIRE(to_entity(entity_2) == 0xfffff);
    REQUIRE(to_version(entity_2) == 0xfff);

    using traits_type = EntityTraits<entity>;
    const auto entity = traits_type::construct(4u, 1u);
    const auto other = traits_type::construct(3u, 0u);
    REQUIRE(to_entity(entity) == 4);
    REQUIRE(to_version(entity) == 1);
    REQUIRE(to_entity(other) == 3);
    REQUIRE(to_version(other) == 0);
    REQUIRE(traits_type::construct(to_entity(entity), to_version(entity)) == entity);
    REQUIRE(traits_type::construct(to_entity(other), to_version(other)) == other);
    REQUIRE(traits_type::construct(to_entity(other), to_version(entity)) ==
            traits_type::combine(to_integral(other), to_integral(entity)));

    constexpr auto empty{empty_entity};
    REQUIRE((empty == empty_entity));
    REQUIRE((empty_entity == empty));
    REQUIRE(empty == entity_2);
    REQUIRE((entity_2 == empty));
    REQUIRE(empty != entity_1);
    REQUIRE((entity_1 != empty));
}