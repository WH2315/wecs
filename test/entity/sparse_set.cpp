#include "wecs/wecs.hpp"
#include "wecs/entity/sparse_set.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace wecs;

#define Entity(x) static_cast<entity>(x)

using SparseSet = BasicSparseSet<entity, config::PageSize>;

TEST_CASE("iterator") {
    //    0  9  2  3
    //  | |        | |
    //  e |        b |
    SparseSet sparse_set;
    REQUIRE(sparse_set.begin() == sparse_set.end());
    sparse_set.insert(Entity(0));
    sparse_set.insert(Entity(9));
    sparse_set.insert(Entity(2));
    sparse_set.insert(Entity(3));
    REQUIRE(sparse_set.size() == 4);

    auto begin = sparse_set.begin();
    REQUIRE(begin == sparse_set.begin());
    REQUIRE(*begin == 3);
    REQUIRE(*begin++ == 3);
    REQUIRE(*begin++ == 2);
    REQUIRE(*begin++ == 9);
    REQUIRE(*begin++ == 0);
    REQUIRE(begin == sparse_set.end());
    REQUIRE(*--begin == 0);
    REQUIRE(*--begin == 9);
    REQUIRE(*--begin == 2);
    REQUIRE(*--begin == 3);
    REQUIRE(begin == sparse_set.begin());
    REQUIRE(*begin == 3);
    REQUIRE(*++begin == 2);
    REQUIRE(*++begin == 9);
    REQUIRE(*++begin == 0);
    REQUIRE(++begin == sparse_set.end());
    REQUIRE(begin-- == sparse_set.end());
    REQUIRE(*begin-- == 0);
    REQUIRE(*begin-- == 9);
    REQUIRE(*begin-- == 2);
    REQUIRE(*begin-- == 3);
    REQUIRE(begin != sparse_set.begin());
    REQUIRE(begin == --sparse_set.begin()); // overflow

    auto end = sparse_set.end();
    REQUIRE(end == sparse_set.end());
    REQUIRE(*--end == 0);
    REQUIRE(*--end == 9);
    REQUIRE(*--end == 2);
    REQUIRE(*--end == 3);
    REQUIRE(end == sparse_set.begin());
    REQUIRE(*end == 3);
    REQUIRE(*end++ == 3);
    REQUIRE(*end++ == 2);
    REQUIRE(*end++ == 9);
    REQUIRE(*end++ == 0);
    REQUIRE(end == sparse_set.end());

    begin = sparse_set.begin();
    REQUIRE(*(begin + 2) == 9);
    end = sparse_set.end();
    REQUIRE(*(end - 2) == 9);

    REQUIRE(sparse_set.packed().back() == 3);
    REQUIRE(sparse_set.find(Entity(0)).index() == 0);
    REQUIRE(sparse_set.find(Entity(9)).index() == 1);
    REQUIRE(sparse_set.find(Entity(2)).index() == 2);
    REQUIRE(sparse_set.find(Entity(3)).index() == 3);
    REQUIRE(sparse_set.find(Entity(100)).index() == std::numeric_limits<size_t>::max());
}

TEST_CASE("insert and remove") {
    SparseSet sparse_set;

    SECTION("insert") {
        REQUIRE(sparse_set.size() == 0);
        REQUIRE(sparse_set.empty());
        sparse_set.insert(Entity(2));
        REQUIRE(sparse_set.size() == 1);
        sparse_set.insert(Entity(4));
        REQUIRE(sparse_set.size() == 2);

        REQUIRE(sparse_set.contain(Entity(2)));
        REQUIRE(sparse_set.contain(Entity(4)));
        REQUIRE_FALSE(sparse_set.contain(Entity(1)));
        REQUIRE_FALSE(sparse_set.contain(Entity(3)));
    }

    SECTION("remove") {
        sparse_set.insert(Entity(2));
        sparse_set.insert(Entity(4));

        REQUIRE(sparse_set.contain(Entity(2)));
        sparse_set.remove(Entity(2));
        REQUIRE_FALSE(sparse_set.contain(Entity(2)));
        REQUIRE(sparse_set.size() == 1);

        sparse_set.remove(Entity(4));
        REQUIRE_FALSE(sparse_set.contain(Entity(4)));
        REQUIRE(sparse_set.empty());

        sparse_set.insert(Entity(1));
        sparse_set.insert(Entity(3));

        sparse_set.clear();
        REQUIRE(sparse_set.size() == 0);
        REQUIRE(sparse_set.empty());
    }
}

TEST_CASE("index") {
    SparseSet sparse_set;

    sparse_set.insert(Entity(1));
    sparse_set.insert(Entity(3));
    sparse_set.insert(Entity(2));
    sparse_set.insert(Entity(4));
    REQUIRE(sparse_set.index(Entity(1)) == 0);
    REQUIRE(sparse_set.index(Entity(3)) == 1);
    REQUIRE(sparse_set.index(Entity(2)) == 2);
    REQUIRE(sparse_set.index(Entity(4)) == 3);

    sparse_set.remove(Entity(3));
    REQUIRE(sparse_set.index(Entity(1)) == 0);
    REQUIRE(sparse_set.index(Entity(3)) == std::numeric_limits<size_t>::max()); // npos
    REQUIRE(sparse_set.index(Entity(2)) == 2);
    REQUIRE(sparse_set.index(Entity(4)) == 1);
}