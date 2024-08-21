#include "renderer.hpp"

#include "sdl_error.hpp"
#include "sdl_header.hpp"
#include "window.hpp"

namespace sdl {

namespace {

SDL_BlendMode to_sdl_blend_mode(BlendMode mode)
{
  switch (mode) {
  case BlendMode::None:
    return SDL_BLENDMODE_NONE;
  case BlendMode::Add:
    return SDL_BLENDMODE_ADD;
  }
};

struct RendererImpl final : public Renderer {
  RendererImpl(SDL_Renderer* ren) : _ren(ren) { assert(_ren != nullptr); }

  virtual ~RendererImpl() { SDL_DestroyRenderer(_ren); }

  static bee::OrError<ptr> create(Window& window, const Attr& attr)
  {
    auto ren = SDL_CreateRenderer(
      window.sdl_window(),
      -1,
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) {
      return EF("SDL_CreateRenderer failed: $", SDL_GetError());
    }

    SDL_SetRenderDrawBlendMode(ren, to_sdl_blend_mode(attr.blend_mode));

    return std::make_shared<RendererImpl>(ren);
  }

  template <class T> SDL_FRect project(const T& rect)
  {
    auto pos = rect.pos.template cast<float>() * _zoom - _view_offset;
    return {
      .x = pos.x,
      .y = pos.y,
      .w = float(rect.size.x) * _zoom,
      .h = float(rect.size.y) * _zoom,
    };
  }

  virtual bee::OrError<> fill_rect(
    const Color& color, const Recti& dst) override
  {
    bail_unit_sdl(
      SDL_SetRenderDrawColor(_ren, color.r, color.g, color.b, color.a) != 0);
    auto sdl_dst_rect = project(dst);
    bail_unit_sdl(SDL_RenderFillRectF(_ren, &sdl_dst_rect) != 0);
    return bee::ok();
  }

  virtual bee::OrError<> fill_rect(
    const Texture& tex, const vec2i& pos) override
  {
    Recti source = {{0, 0}, tex.size()};
    Recti dest = {pos, tex.size()};
    return fill_rect(tex, source, dest);
  }

  virtual bee::OrError<> fill_rect(
    const Texture& texture, const Recti& source, const Recti& dest) override
  {
    SDL_Rect src_rect{
      .x = source.pos.x,
      .y = source.pos.y,
      .w = source.size.x,
      .h = source.size.y,
    };

    auto dst_rect = project(dest);

    bail_unit_sdl(
      SDL_RenderCopyF(_ren, texture.sdl_texture(), &src_rect, &dst_rect));
    return bee::ok();
  }

  virtual bee::OrError<> fill_rect(
    const Texture& texture,
    const Recti& source,
    const Rectf& dest,
    double angle) override
  {
    SDL_Rect src_rect{
      .x = source.pos.x,
      .y = source.pos.y,
      .w = source.size.x,
      .h = source.size.y,
    };

    auto dst_rect = project(dest);

    bail_unit_sdl(SDL_RenderCopyExF(
      _ren,
      texture.sdl_texture(),
      &src_rect,
      &dst_rect,
      angle,
      nullptr,
      SDL_FLIP_NONE));
    return bee::ok();
  }

  virtual bee::OrError<> fill_rect(
    const Texture& texture, const Recti& dest) override
  {
    return fill_rect(texture, Recti{{0, 0}, texture.size()}, dest);
  }

  bee::OrError<> fill_all(const pixel::Image& img) override
  {
    bail(tex, Texture::create_from_image(_ren, img));
    return fill_rect(*tex, viewport());
  }

  virtual void present() override { SDL_RenderPresent(_ren); }
  virtual bee::OrError<> clear() override
  {
    bail_unit_sdl(SDL_SetRenderDrawColor(_ren, 0, 0, 0, 255));
    bail_unit_sdl(SDL_RenderClear(_ren));
    return bee::ok();
  }

  virtual void set_view(const vec2f& offset) override { _view_offset = offset; }

  virtual void set_zoom(float zoom) override { _zoom = zoom; }

  virtual const vec2f& view_offset() const override { return _view_offset; }

  virtual bee::OrError<Texture::ptr> create_texture(
    const RawImage& img) override
  {
    return Texture::create_from_raw_image(_ren, img);
  }

  virtual Recti viewport() const override
  {
    SDL_Rect sdl_rect;
    SDL_RenderGetViewport(_ren, &sdl_rect);
    return {{sdl_rect.x, sdl_rect.y}, {sdl_rect.w, sdl_rect.h}};
  }

  virtual vec2i output_size() const override
  {
    int w, h;
    SDL_GetRendererOutputSize(_ren, &w, &h);
    return {w, h};
  }

  virtual SDL_Renderer* sdl_renderer() override { return _ren; }

  SDL_Renderer* _ren;

  vec2f _view_offset = {0, 0};
  float _zoom = 1.0;
};

} // namespace

Renderer::~Renderer() {}

bee::OrError<Renderer::ptr> Renderer::create(Window& window, const Attr& attr)
{
  return RendererImpl::create(window, attr);
}

} // namespace sdl
