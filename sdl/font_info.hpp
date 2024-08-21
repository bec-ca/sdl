#pragma once

#include "bee/file_path.hpp"

namespace sdl {

struct FontInfo {
 public:
  ~FontInfo();

  bee::FilePath file_path;
  std::string name;
  std::string style;

  std::string to_string() const;
};

} // namespace sdl
