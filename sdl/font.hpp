#pragma once

#include <memory>

#include "font_info.hpp"
#include "renderer.hpp"
#include "texture.hpp"

#include "bee/or_error.hpp"

namespace sdl {

struct Font {
 public:
  using ptr = std::shared_ptr<Font>;

  virtual ~Font();

  virtual bee::OrError<Texture::ptr> render_text(
    Renderer& ren, const std::string& text) = 0;

  static bee::OrError<ptr> create(int size);

  virtual const FontInfo& info() const = 0;
};

struct TTF {
  static bee::OrError<> init();
};

} // namespace sdl
