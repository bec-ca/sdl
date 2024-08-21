#include "font_info.hpp"

#include "bee/format.hpp"

namespace sdl {

FontInfo::~FontInfo() {}

std::string FontInfo::to_string() const
{
  return F("$:$:$", file_path, name, style);
}

} // namespace sdl
