#include "wecs/signal/delegate.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace wecs;

int power_of_two(const int& i) {
    return i * i;
}

struct delegate_functor {
    int operator()(int i) {
        return i + i;
    }

    int identity(int i) const {
        return i;
    }

    static const int static_value = 3;
    // NOLINTNEXTLINE(*-avoid-const-or-ref-data-members)
    const int data_member = 4;
};

TEST_CASE("Functionalities") {
    Delegate<int(int)> ff;
    Delegate<int(int)> mf;
    Delegate<int(int)> lf;
    delegate_functor functor;

    ff.connect<&power_of_two>();
    mf.connect<&delegate_functor::operator()>(functor);
    lf.connect(
        [](const void* ptr, int value) {
            return static_cast<const delegate_functor*>(ptr)->identity(value);
        },
        &functor);

    REQUIRE(ff(3) == 9);
    REQUIRE(mf(3) == 6);
    REQUIRE(lf(3) == 3);
}

TEST_CASE("DataMembers") {
    Delegate<double()> delegate;
    delegate_functor functor;
    delegate.connect<&delegate_functor::data_member>(&functor);
    REQUIRE(delegate() == 4);
}