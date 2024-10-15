#pragma once

namespace wecs {

struct TypeInfo {};

template <typename T>
TypeInfo* get_type_info() {
    static TypeInfo type_info;
    return &type_info;
}

} // namespace wecs