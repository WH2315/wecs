#pragma once

#include <cstddef>

namespace wecs {

template <typename... Type>
struct TypeList {
    using type = TypeList;
    static constexpr size_t size = sizeof...(Type);
};

template <size_t, typename>
struct TypeListElement;

template <typename T, typename... Ts, size_t Index>
struct TypeListElement<Index, TypeList<T, Ts...>>
    : TypeListElement<Index - 1u, TypeList<Ts...>> {};

template <typename T, typename... Ts>
struct TypeListElement<0u, TypeList<T, Ts...>> {
    using type = T;
};

template <size_t Index, typename List>
using type_list_element_t = typename TypeListElement<Index, List>::type;

} // namespace wecs