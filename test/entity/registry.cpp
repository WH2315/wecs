#include "wecs/wecs.hpp"
#include "wecs/entity/registry.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace wecs;

#define Entity(x) static_cast<entity>(x)

using Registry = BasicRegistry<entity, config::page_size>;

struct Component1 {
    int a;
};

struct Component2 {
    std::string str;
};

struct Component3 {
    float f;
};

struct Component4 {
    double d;
};

TEST_CASE("registry") {
    Registry registry;

    SECTION("entities") {
        auto entity_0 = registry.create();
        REQUIRE(entity_0 == Entity(0));
        auto entity_1 = registry.create();
        REQUIRE(entity_1 == Entity(1));

        REQUIRE(registry.alive(entity_0));
        REQUIRE(registry.alive(entity_1));

        registry.destroy(entity_0);
        REQUIRE_FALSE(registry.alive(entity_0));

        registry.clear();
        REQUIRE(registry.size() == 0);
    }

    SECTION("emplace") {
        auto entity_1 = registry.create();
        auto entity_2 = registry.create();

        auto& comp_1 = registry.emplace<Component1>(entity_1, Component1{10});
        REQUIRE(comp_1.a == 10);
        REQUIRE(registry.has<Component1>(entity_1));

        auto& comp_2 = registry.emplace<Component2>(entity_1, Component2{"wang"});
        REQUIRE(comp_2.str == "wang");
        REQUIRE(registry.has<Component2>(entity_1));

        REQUIRE_FALSE(registry.has<Component1>(entity_2));
        REQUIRE_FALSE(registry.has<Component2>(entity_2));

        REQUIRE(registry.get<Component1>(entity_1).a == 10);
        REQUIRE(registry.get<Component2>(entity_1).str == "wang");
    }

    SECTION("patch") {
        auto entity_1 = registry.create();

        auto& comp_1 = registry.emplace<Component1>(entity_1, Component1{10});
        REQUIRE(comp_1.a == 10);
        REQUIRE(registry.has<Component1>(entity_1));

        auto& comp_2 = registry.patch<Component1>(entity_1, [](Component1& comp) {
            comp.a = 100;
        });
        REQUIRE(comp_2.a == 100);
        REQUIRE(registry.has<Component1>(entity_1));
    }

    SECTION("replace") {
        auto entity_1 = registry.create();

        auto& comp_1 = registry.emplace<Component1>(entity_1, Component1{10});
        REQUIRE(comp_1.a == 10);
        REQUIRE(registry.has<Component1>(entity_1));

        auto& comp_2 = registry.replace<Component1>(entity_1, Component1{100});
        REQUIRE(comp_2.a == 100);
        REQUIRE(registry.has<Component1>(entity_1));
    }

    SECTION("view") {
        auto entity_1 = registry.create();
        auto entity_2 = registry.create();
        auto entity_3 = registry.create();
        auto entity_4 = registry.create();

        registry.emplace<Component1>(entity_1, Component1{10});
        registry.emplace<Component1>(entity_2, Component1{20});
        registry.emplace<Component1>(entity_3, Component1{30});
        registry.emplace<Component2>(entity_1, Component2{"abc"});
        registry.emplace<Component2>(entity_2, Component2{"def"});
        registry.emplace<Component3>(entity_2, Component3{3.14f});
        registry.emplace<Component4>(entity_4, Component4{2.718});
        
        auto view_1 = registry.view<Component1>();
        for (auto&& [entity, comp] : view_1) {
            std::cout << (uint32_t)entity << ": " << comp.a << std::endl;
            comp.a += 1;
            std::cout << (uint32_t)entity << ": " << comp.a << std::endl;
        }
        auto view_2 = registry.view<Component1, Component2>();
        for (auto&& [entity, comp1, comp2] : view_2) {
            std::cout << (uint32_t)entity << ": " << comp1.a << ", " << comp2.str << std::endl;
        }
        auto view_3 = registry.view<Component3, Component2, Component1>();
        for (auto&& [entity, comp3, comp2, comp1] : view_3) {
            std::cout << (uint32_t)entity << ": " << comp1.a << ", " << comp2.str << ", " << comp3.f << std::endl;
        }
        auto view_4 = registry.view<Component1, Component4>();
        for (auto&& [entity, comp1, comp4] : view_4) {
            assert(false);
        } 
    }
}