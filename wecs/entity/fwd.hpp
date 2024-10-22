#pragma once

#include "wecs/entity/registry.hpp"

namespace wecs {

using registry = BasicRegistry<config::Entity, config::page_size>;

} // namespace wecs