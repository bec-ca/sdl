#pragma once

#include "font_info.hpp"

#include "bee/file_path.hpp"
#include "bee/or_error.hpp"

namespace sdl {

struct FontLocator {
  static bee::OrError<FontInfo> find_font();
};

} // namespace sdl
