#include "texture.hpp"

#include "sdl_header.hpp"

#include "pixel/image.hpp"

namespace sdl {
namespace {

struct TextureImpl : public Texture {
 public:
  virtual ~TextureImpl() { SDL_DestroyTexture(_tex); }

  TextureImpl(const vec2i& size, SDL_Texture* tex) : _size(size), _tex(tex) {}

  virtual SDL_Texture* sdl_texture() const override { return _tex; }

  virtual const vec2i& size() const override { return _size; }

 private:
  const vec2i _size;
  SDL_Texture* _tex;
};

} // namespace

Texture::~Texture() {}

bee::OrError<Texture::ptr> Texture::create_from_sdl_surface(
  SDL_Renderer* ren, SDL_Surface* surface, bool enable_alpha_blend)
{
  auto width = surface->w;
  auto height = surface->h;

  auto texture = SDL_CreateTextureFromSurface(ren, surface);
  SDL_FreeSurface(surface);
  if (texture == nullptr) {
    return bee::Error::fmt(
      "Failed to create texture from surface: $", SDL_GetError());
  }

  if (enable_alpha_blend) {
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD);
  }

  return std::make_unique<TextureImpl>(vec2i{width, height}, texture);
}

bee::OrError<Texture::ptr> Texture::create_from_raw_image(
  SDL_Renderer* ren, const RawImage& img)
{
  auto surface = SDL_CreateRGBSurfaceFrom(
    (void*)img.pixel_data.data(),
    img.width,
    img.height,
    8 * img.bytes_per_pixel,
    img.bytes_per_pixel * img.width,
    0,
    0,
    0,
    0);

  return create_from_sdl_surface(ren, surface, img.bytes_per_pixel == 4);
}

bee::OrError<Texture::ptr> Texture::create_from_image(
  SDL_Renderer* ren, const pixel::Image& img)
{
  auto surface = SDL_CreateRGBSurfaceFrom(
    (void*)img.data(),
    img.width(),
    img.height(),
    8 * 3,
    3 * img.width(),
    0xff,
    0xff00,
    0xff0000,
    0);

  return create_from_sdl_surface(ren, surface, false);
}

} // namespace sdl
