#pragma once

#include <cstdint>
#include <cassert>

#define WECS_ASSERT(condition, msg) assert(((condition) && (msg)))

#ifndef SPARSE_PAGE_SIZE
#define SPARSE_PAGE_SIZE 4096
#endif

#ifndef ENTITY_NUMERIC_TYPE
#define ENTITY_NUMERIC_TYPE uint32_t
#endif

namespace wecs::config {

enum class Entity : ENTITY_NUMERIC_TYPE {};
constexpr uint32_t PageSize = SPARSE_PAGE_SIZE;

} // namespace wecs::config