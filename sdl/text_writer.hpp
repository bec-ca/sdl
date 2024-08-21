#pragma once

#include <memory>

#include "renderer.hpp"

#include "bee/or_error.hpp"

namespace sdl {

struct TextWriter {
 public:
  using ptr = std::unique_ptr<TextWriter>;

  TextWriter(Texture::ptr&& tex);
  ~TextWriter();

  static bee::OrError<ptr> create(Renderer& renderer);

  bee::OrError<> draw_text(
    Renderer& ren, const vec2i& pos, const std::string& text);

 private:
  Texture::ptr _tex;
};

} // namespace sdl
