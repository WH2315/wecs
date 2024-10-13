#include "wecs/wecs.hpp"
#include "wecs/entity/storage.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace wecs;

#define Entity(x) static_cast<entity>(x)

TEST_CASE("pure storage") {
    BasicStorage<entity, entity, config::PageSize, void> storage;
    REQUIRE(storage.size() == 0);
    REQUIRE(storage.base_size() == 0);

    auto my_entity = storage.emplace();
    REQUIRE(my_entity == Entity(0));
    REQUIRE(storage.size() == 1);
    REQUIRE(storage.base_size() == 1);
    my_entity = storage.emplace();
    REQUIRE(my_entity == Entity(1));
    REQUIRE(storage.size() == 2);
    REQUIRE(storage.base_size() == 2);

    storage.remove(Entity(1));
    REQUIRE(storage.size() == 1);
    REQUIRE(storage.base_size() == 2);

    my_entity = storage.emplace();
    REQUIRE(to_integral(my_entity) == 0x00100001);
    REQUIRE(storage.size() == 2);
    REQUIRE(storage.base_size() == 2);

    REQUIRE(storage.contain(Entity(0)));
    REQUIRE(storage.contain(Entity(0x100001)));
    REQUIRE(storage.contain(my_entity));
}

struct Point {
    float x, y;

    bool operator==(const Point& o) const {
        return x == o.x && y == o.y;
    }
};

TEST_CASE("storage") {
    BasicStorage<entity, Point, config::PageSize, std::allocator<Point>> storage;
    REQUIRE(storage.size() == 0);
    REQUIRE(storage.empty());
    REQUIRE_FALSE(storage.contain(Entity(2)));
    REQUIRE(storage.begin() == storage.end());

    auto p1 = storage.emplace(Entity(0), Point{1.0, 3.0});
    REQUIRE(p1.x == 1.0);
    REQUIRE(p1.y == 3.0);
    REQUIRE(storage.size() == 1);
    REQUIRE(storage.begin()->x == 1.0);
    REQUIRE(storage.begin()->y == 3.0);

    auto p2 = storage.emplace(Entity(2), Point{4.0, 7.0});
    REQUIRE(p2.x == 4.0);
    REQUIRE(p2.y == 7.0);
    REQUIRE(storage.size() == 2);
    REQUIRE(*storage.begin() == p2);
    REQUIRE(*(storage.begin() + 1) == p1);


    auto p3 = storage.patch(Entity(2), [](Point& p) {
        p.x = 5.0;
    });
    REQUIRE(p3.x == 5.0);
    REQUIRE(p3.y == 7.0);
    REQUIRE(storage.size() == 2);
    REQUIRE(*storage.begin() == p3);
    REQUIRE(*(storage.begin() + 1) == p1);

    auto p4 = storage.emplace(Entity(4), Point{7.0, 11.0});

    storage.remove(Entity(0));
    REQUIRE(storage.size() == 2);

    storage.clear();
    REQUIRE(storage.empty());
}