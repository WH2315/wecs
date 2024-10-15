#pragma once

#include "wecs/config/type_info.hpp"
#include <cstdint>
#include <cassert>

#define WECS_ASSERT(condition, msg) assert(((condition) && (msg)))

#ifndef SPARSE_PAGE_SIZE
#define SPARSE_PAGE_SIZE 4096
#endif

#ifndef ENTITY_NUMERIC_TYPE
#define ENTITY_NUMERIC_TYPE uint32_t
#endif

#ifndef GET_TYPE_INFO
#define GET_TYPE_INFO(type) get_type_info<type>()
#endif

namespace wecs::config {

enum class Entity : ENTITY_NUMERIC_TYPE {};
constexpr uint32_t page_size = SPARSE_PAGE_SIZE;
using type_info = const TypeInfo*;
using id_type = uint32_t;

} // namespace wecs::config