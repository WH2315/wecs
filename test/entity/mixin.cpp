#include "wecs/wecs.hpp"
#include "wecs/entity/mixin.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace wecs;

#define Entity(x) static_cast<entity>(x)

struct Point {
    float x, y;
};

void construct() {
    std::cout << "construct\n";
}

TEST_CASE("mixin") {
    using Storage = BasicStorage<entity, Point, config::page_size, std::allocator<Point>>;
    SighMixin<Storage> mixin;

    mixin.on_construct().connect<&construct>();
    auto& p1 = mixin.emplace(Entity(0), 1.0f, 3.0f);
    REQUIRE(p1.x == 1.0f);
    REQUIRE(p1.y == 3.0f);
}