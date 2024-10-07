#pragma once

#include <type_traits>

namespace wecs {

namespace internal {

template <typename Type, typename = void>
struct PageSize : std::integral_constant<size_t, !std::is_empty_v<Type>> {};

template <>
struct PageSize<void> : std::integral_constant<size_t, 0u> {};

template <typename Type>
struct PageSize<Type, std::void_t<decltype(Type::page_size)>>
    : std::integral_constant<size_t, Type::page_size> {};

} // namespace internal

template <typename Type, typename = void>
struct ComponentTraits {
    static_assert(std::is_same_v<std::decay_t<Type>, Type>, "unsupported type");

    using type = Type;

    static constexpr size_t page_size = internal::PageSize<type>::value;
};

} // namespace wecs