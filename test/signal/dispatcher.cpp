#include "wecs/signal/dispatcher.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace wecs;

struct Event {
    int a = 20;
};

int count = 0;
void fun(const Event& e) {
    count += e.a;
}

TEST_CASE("dispatcher") {
    Dispatcher dispatcher;

    dispatcher.sink<Event>().connect<&fun>();

    dispatcher.trigger(Event{});
    REQUIRE(count == 20);
    dispatcher.trigger<Event>(100);
    REQUIRE(count == 120);

    dispatcher.enqueue(Event{1});
    dispatcher.enqueue<Event>(2);
    dispatcher.update();
    REQUIRE(count == 123);
}