#pragma once

#include <memory>

#include "raw_image.hpp"
#include "rect.hpp"
#include "sdl_types.hpp"

#include "bee/or_error.hpp"
#include "pixel/image.hpp"

namespace sdl {

struct Texture {
 public:
  using ptr = std::unique_ptr<Texture>;

  virtual ~Texture();

  virtual const vec2i& size() const = 0;

  virtual SDL_Texture* sdl_texture() const = 0;

  static bee::OrError<ptr> create_from_raw_image(
    SDL_Renderer* ren, const RawImage& img);

  static bee::OrError<ptr> create_from_sdl_surface(
    SDL_Renderer* ren, SDL_Surface* sdl_surface, bool enable_alpha_blend);

  static bee::OrError<Texture::ptr> create_from_image(
    SDL_Renderer* ren, const pixel::Image& img);
};

} // namespace sdl
