#pragma once

#include "wecs/config/config.hpp"
#include <unordered_map>

namespace wecs {

template <typename T>
struct Ident {
    using value_type = config::id_type;

    inline static std::unordered_map<config::type_info, value_type> map;
    inline static value_type index{};

    template <typename Type>
    static value_type get() noexcept {
        config::type_info type_info = GET_TYPE_INFO(Type);
        if (auto it = map.find(type_info); it != map.end()) {
            return it->second;
        }
        auto idx = index++;
        map.emplace(type_info, idx);
        return idx;
    }
};

} // namespace wecs