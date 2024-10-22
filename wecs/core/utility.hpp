#pragma once

#include <type_traits>

namespace wecs {

template <typename To, typename From>
struct constness_as {
    using type = std::remove_const_t<To>;
};

template <typename To, typename From>
struct constness_as<To, const From> {
    using type = const To;
};

template <typename To, typename From>
using constness_as_t = typename constness_as<To, From>::type;

} // namespace wecs