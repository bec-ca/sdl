#pragma once

#include <memory>

#include "sdl_context.hpp"
#include "sdl_types.hpp"
#include "vec2.hpp"

#include "bee/or_error.hpp"
#include "pixel/image.hpp"

namespace sdl {

struct Window {
 public:
  using ptr = std::unique_ptr<Window>;

  virtual ~Window();

  static bee::OrError<ptr> create(
    const SDLContext& ctx, const std::string& title, const vec2i& size);

  virtual SDL_Window* sdl_window() = 0;

  virtual vec2i size() const = 0;

  virtual void set_size(const vec2i& new_size) = 0;

  [[nodiscard]] virtual bee::OrError<> blit(const pixel::Image& img) = 0;
};

} // namespace sdl
