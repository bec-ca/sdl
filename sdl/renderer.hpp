#pragma once

#include <memory>

#include "color.hpp"
#include "rect.hpp"
#include "texture.hpp"
#include "window.hpp"

#include "bee/or_error.hpp"

namespace sdl {

enum class BlendMode {
  None,
  Add,
};

struct Renderer {
 public:
  using ptr = std::shared_ptr<Renderer>;

  struct Attr {
    BlendMode blend_mode = BlendMode::None;
  };

  virtual ~Renderer();

  [[nodiscard]] virtual bee::OrError<> fill_rect(
    const Color& color, const Recti& rect) = 0;

  [[nodiscard]] virtual bee::OrError<> fill_rect(
    const Texture& texture, const vec2i& pos) = 0;

  [[nodiscard]] virtual bee::OrError<> fill_rect(
    const Texture& texture, const Recti& dest) = 0;

  [[nodiscard]] virtual bee::OrError<> fill_rect(
    const Texture& texture, const Recti& source, const Recti& dest) = 0;

  [[nodiscard]] virtual bee::OrError<> fill_rect(
    const Texture& texture,
    const Recti& source,
    const Rectf& dest,
    double angle) = 0;

  [[nodiscard]] virtual bee::OrError<> fill_all(const pixel::Image& img) = 0;

  virtual void present() = 0;
  virtual bee::OrError<> clear() = 0;

  virtual void set_view(const vec2f& offset) = 0;
  virtual void set_zoom(float zoom) = 0;

  virtual const vec2f& view_offset() const = 0;

  virtual Recti viewport() const = 0;

  virtual vec2i output_size() const = 0;

  virtual SDL_Renderer* sdl_renderer() = 0;

  virtual bee::OrError<Texture::ptr> create_texture(const RawImage& img) = 0;

  static bee::OrError<ptr> create(Window&, const Attr& attr);
};

} // namespace sdl
