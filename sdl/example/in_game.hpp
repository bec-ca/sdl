#pragma once

#include "controller.hpp"
#include "level.hpp"

namespace sdl::example {

struct InGame {
  static Controller::ptr create(std::optional<Level>&& level);
};

} // namespace sdl::example
