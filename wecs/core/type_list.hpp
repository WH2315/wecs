#pragma once

#include <tuple>
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

template <typename List>
struct TypeListToTuple;

template <typename... Ts>
struct TypeListToTuple<TypeList<Ts...>> {
    using type = std::tuple<Ts...>;
};

template <typename List>
using type_list_to_tuple_t = typename TypeListToTuple<List>::type;

template <typename List, template <typename...> class Op>
struct TypeListTransform;

template <typename... Ts, template <typename...> class Op>
struct TypeListTransform<TypeList<Ts...>, Op> {
    using type = TypeList<typename Op<Ts>::type...>;
};

template <typename List, template <typename> class Op>
using type_list_transform_t = typename TypeListTransform<List, Op>::type;

} // namespace wecs