#include "wecs/signal/sigh.hpp"
#include "wecs/signal/sink.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace wecs;

struct sigh_listener {
    static void f(int& v) { ++v; }

    bool g(int) {
        val = !val;
        return true;
    }

    bool h(const int&) { return val; }

    // useless definition just because msvc does weird things if both are empty
    void i() { val = true && val; }

    bool val{false};
};

void connect_and_auto_disconnect(Sigh<void(int&)>& sigh, const int&) {
    Sink sink{sigh};
    sink.connect<sigh_listener::f>();
    sink.disconnect<&connect_and_auto_disconnect>(sigh);
}

TEST_CASE("sink") {
    SECTION("Disconnect") {
        sigh_listener listener;
        Sigh<void(int&)> sigh;
        Sink sink{sigh};

        sink.connect<&sigh_listener::f>();
        REQUIRE_FALSE(sink.empty());
        REQUIRE_FALSE(sigh.empty());
        sink.disconnect<&sigh_listener::f>();
        REQUIRE(sink.empty());
        REQUIRE(sigh.empty());

        sink.connect<&sigh_listener::g>(listener);
        REQUIRE_FALSE(sink.empty());
        REQUIRE_FALSE(sigh.empty());
        sink.disconnect(&listener);
        REQUIRE(sink.empty());
        REQUIRE(sigh.empty());

        sink.connect<&sigh_listener::f>();
        REQUIRE_FALSE(sink.empty());
        REQUIRE_FALSE(sigh.empty());
        sink.disconnect();
        REQUIRE(sink.empty());
        REQUIRE(sigh.empty());
    }

    SECTION("Functions") {
        Sigh<void(int&)> sigh;
        Sink sink{sigh};
        int v = 0;

        sink.connect<&sigh_listener::f>();
        sigh.trigger(v);
        REQUIRE(sigh.size() == 1);
        REQUIRE(v == 1);

        v = 0;
        sink.disconnect<&sigh_listener::f>();
        sigh.trigger(v);
        REQUIRE(sigh.empty());
        REQUIRE(v == 0);
    }

    SECTION("Functions with Payload") {
        Sigh<void()> sigh;
        Sink sink{sigh};
        int v = 0;

        sink.connect<&sigh_listener::f>(v);
        sigh.trigger();
        REQUIRE(sigh.size() == 1);
        REQUIRE(v == 1);

        v = 0;
        sink.disconnect<&sigh_listener::f>(v);
        sigh.trigger();
        REQUIRE(sigh.empty());
        REQUIRE(v == 0);

        sink.connect<&sigh_listener::f>(v);
        sink.disconnect(&v);
        sigh.trigger();
        REQUIRE(v == 0);
    }

    SECTION("Members") {
        sigh_listener l1, l2;
        Sigh<bool(int)> sigh;
        Sink sink{sigh};

        sink.connect<&sigh_listener::g>(l1);
        sigh.trigger(3);
        REQUIRE(l1.val);

        sink.disconnect<&sigh_listener::g>(l1);
        sigh.trigger(3);
        REQUIRE(l1.val);

        sink.connect<&sigh_listener::g>(&l1);
        sink.connect<&sigh_listener::h>(l2);
        REQUIRE(sigh.size() == 2);

        sink.disconnect(&l1);
        REQUIRE(sigh.size() == 1);
    }

    SECTION("Connection") {
        Sigh<void(int&k)> sigh;
        Sink sink{sigh};
        int v = 0;
        
        auto coon = sink.connect<&sigh_listener::f>();
        sigh.trigger(v);
        REQUIRE(coon);
        REQUIRE(v == 1);

        v = 0;
        coon.release();
        sigh.trigger(v);
        REQUIRE_FALSE(coon);
        REQUIRE(v == 0);
    }

    SECTION("Connect and auto Disconnect") {
        sigh_listener listener;
        Sigh<void(int&)> sigh;
        Sink sink{sigh};
        int v = 0;

        sink.connect<&sigh_listener::g>(listener);
        sink.connect<&connect_and_auto_disconnect>(sigh);

        REQUIRE(sigh.size() == 2);
        REQUIRE(listener.val == false);
        REQUIRE(v == 0);

        sigh.trigger(v);
        REQUIRE(sigh.size() == 2);
        REQUIRE(listener.val == true);
        REQUIRE(v == 0);

        sigh.trigger(v);
        REQUIRE(sigh.size() == 2);
        REQUIRE(listener.val == false);
        REQUIRE(v == 1);
    }
}