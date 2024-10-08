#pragma once

#include "wecs/core/type_list.hpp"
#include "wecs/core/utility.hpp"
#include "wecs/config/config.hpp"
#include <tuple>
#include <utility>
#include <functional>

namespace wecs {

namespace internal {

template <typename Ret, typename... Args>
constexpr auto function_pointer(Ret (*)(Args...)) -> Ret (*)(Args...);

template <typename Ret, typename Type, typename... Args, typename Other>
constexpr auto function_pointer(Ret (*)(Type, Args...), Other&&) -> Ret (*)(Args...);

template <typename Class, typename Ret, typename... Args, typename... Other>
constexpr auto function_pointer(Ret (Class::*)(Args...), Other&&...) -> Ret (*)(Args...);

template <typename Class, typename Ret, typename... Args, typename... Other>
constexpr auto function_pointer(Ret (Class::*)(Args...) const, Other&&...) -> Ret (*)(Args...);

template <typename Class, typename Type, typename... Other, typename = std::enable_if_t<std::is_member_object_pointer_v<Type Class::*>>>
constexpr auto function_pointer(Type Class::*, Other&&...) -> Type (*)();

template <typename... Type>
using function_pointer_t = decltype(function_pointer(std::declval<Type>()...));

template <typename... Class, typename Ret, typename... Args>
constexpr auto index_sequence_for(Ret (*)(Args...)) {
    return std::index_sequence_for<Class..., Args...>{};
}

} // namespace internal

template <typename Function>
class Delegate;

template <typename Ret, typename... Args>
class Delegate<Ret(Args...)> {
public:
    using function_type = Ret(const void*, Args...);

    Delegate() = default;
    ~Delegate() { reset(); }

    template <auto Function>
    void connect() {
        instance_ = nullptr;
        if constexpr (std::is_invocable_r_v<Ret, decltype(Function), Args...>) {
            function_ = [](const void*, Args... args) -> Ret {
                return Ret(std::invoke(Function, std::forward<Args>(args)...));
            };
        } else if constexpr (std::is_member_pointer_v<decltype(Function)>) {
            function_ = wrap<Function>(internal::index_sequence_for<type_list_element_t<0, TypeList<Args...>>>(internal::function_pointer_t<decltype(Function)>{}));
        } else {
            function_ = wrap<Function>(internal::index_sequence_for(internal::function_pointer_t<decltype(Function)>{}));
        }
    }

    template <auto Function, typename Payload>
    void connect(Payload& payload) {
        instance_ = &payload;
        if constexpr(std::is_invocable_r_v<Ret, decltype(Function), Payload&, Args...>) {
            function_ = [](const void* payload, Args... args) -> Ret {
                Payload* curr = static_cast<Payload*>(const_cast<constness_as_t<void, Payload>*>(payload));
                return Ret(std::invoke(Function, *curr, std::forward<Args>(args)...));
            };
        } else {
            function_ = wrap<Function>(payload, internal::index_sequence_for(internal::function_pointer_t<decltype(Function), Payload>{}));
        }
    }

    template <auto Function, typename Payload>
    void connect(Payload* payload) {
        instance_ = payload;
        if constexpr(std::is_invocable_r_v<Ret, decltype(Function), Payload*, Args...>) {
            function_ = [](const void* payload, Args... args) -> Ret {
                Payload* curr = static_cast<Payload*>(const_cast<constness_as_t<void, Payload>*>(payload));
                return Ret(std::invoke(Function, curr, std::forward<Args>(args)...));
            };
        } else {
            function_ = wrap<Function>(payload, internal::index_sequence_for(internal::function_pointer_t<decltype(Function), Payload>{}));
        }
    }

    void connect(function_type* function, const void* payload = nullptr) {
        WECS_ASSERT(function != nullptr, "uninitialized function pointer");
        instance_ = payload;
        function_ = function;
    }

public:
    Ret operator()(Args... args) const {
        return function_(instance_, std::forward<Args>(args)...);
    }

    function_type* target() const {
        return function_;
    }

    const void* data() const {
        return instance_;
    }

    operator bool() const {
        return function_ != nullptr;
    }

    bool operator==(const Delegate& other) const {
        return other.instance_ == instance_ && other.function_ == function_;
    }

    bool operator!=(const Delegate& other) const {
        return !(*this == other);
    }

    void reset() {
        instance_ = nullptr;
        function_ = nullptr;
    }

private:
    template <auto Function, size_t... Index>
    auto wrap(std::index_sequence<Index...>) {
        using args_list = TypeList<Args...>;
        return [](const void*, Args... args) -> Ret {
            const auto arguments = std::forward_as_tuple(std::forward<Args>(args)...);
            constexpr auto offset = !std::is_invocable_r_v<Ret, decltype(Function), type_list_element_t<Index, args_list>...> * (sizeof...(Args) - sizeof...(Index));
            return static_cast<Ret>(std::invoke(Function, std::forward<type_list_element_t<Index + offset, args_list>>(std::get<Index + offset>(arguments))...));
        };
    }

    template <auto Function, typename Payload, size_t... Index>
    auto wrap(Payload&, std::index_sequence<Index...>) {
        using args_list = TypeList<Args...>;
        return [](const void* payload, Args... args) -> Ret {
            Payload* curr = static_cast<Payload*>(const_cast<constness_as_t<void, Payload>*>(payload));
            const auto arguments = std::forward_as_tuple(std::forward<Args>(args)...);
            constexpr auto offset = !std::is_invocable_r_v<Ret, decltype(Function), Payload&, type_list_element_t<Index, args_list>...> * (sizeof...(Args) - sizeof...(Index));
            return static_cast<Ret>(std::invoke(Function, *curr, std::forward<type_list_element_t<Index + offset, args_list>>(std::get<Index + offset>(arguments))...));
        };
    }

    template <auto Function, typename Payload, size_t... Index>
    auto wrap(Payload*, std::index_sequence<Index...>) {
        using args_list = TypeList<Args...>;
        return [](const void* payload, Args... args) -> Ret {
            Payload* curr = static_cast<Payload*>(const_cast<constness_as_t<void, Payload>*>(payload));
            const auto arguments = std::forward_as_tuple(std::forward<Args>(args)...);
            constexpr auto offset = !std::is_invocable_r_v<Ret, decltype(Function), Payload*, type_list_element_t<Index, args_list>...> * (sizeof...(Args) - sizeof...(Index));
            return static_cast<Ret>(std::invoke(Function, curr, std::forward<type_list_element_t<Index + offset, args_list>>(std::get<Index + offset>(arguments))...));
        };
    }

private:
    const void* instance_{};
    function_type* function_{};
};

} // namespace wecs