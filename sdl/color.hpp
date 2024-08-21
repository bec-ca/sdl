#pragma once

#include <cstdint>

#include "sdl_header.hpp"

namespace sdl {

struct Color {
  uint8_t r, g, b, a;

  constexpr static Color white() { return {255, 255, 255, 255}; }
  constexpr static Color black() { return {0, 0, 0, 255}; }

  SDL_Color to_sdl_color() const { return {r, g, b, a}; }
};

} // namespace sdl
