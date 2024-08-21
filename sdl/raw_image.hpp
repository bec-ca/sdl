#pragma once

#include <string>

namespace sdl {

struct RawImage {
  const int width;
  const int height;
  const int bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  const std::string pixel_data;
};

struct Images {
  static RawImage Squares;
  static RawImage Letters;
};

} // namespace sdl
