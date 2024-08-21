#pragma once

namespace sdl {}

#define bail_unit_sdl(expr)                                                    \
  if ((expr) != 0) {                                                           \
    return bee::Error::fmt(#expr " failed: $", SDL_GetError());                \
  }
