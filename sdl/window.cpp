#include "window.hpp"

#include "sdl_header.hpp"
#include "vec2.hpp"

#include "pixel/image.hpp"

namespace sdl {

namespace {

struct WindowImpl final : public Window {
  WindowImpl(SDL_Window* win) : _win(win) { assert(_win != nullptr); }

  virtual ~WindowImpl() { SDL_DestroyWindow(_win); }

  virtual SDL_Window* sdl_window() override { return _win; }

  static bee::OrError<ptr> create(
    const SDLContext&, const std::string& title, const vec2i& size)
  {
    auto win = SDL_CreateWindow(
      title.c_str(),
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      size.x,
      size.y,
      SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_ALWAYS_ON_TOP);
    if (win == nullptr) {
      return bee::Error::fmt("SDL_CreateWindow failed: $", SDL_GetError());
    }
    return std::make_unique<WindowImpl>(win);
  }

  virtual vec2i size() const override
  {
    vec2i size;
    SDL_GetWindowSize(_win, &size.x, &size.y);
    return size;
  }

  virtual void set_size(const vec2i& size) override
  {
    SDL_SetWindowSize(_win, size.x, size.y);
  }

  bee::OrError<> blit(const pixel::Image& img) override
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

    auto window_surf = SDL_GetWindowSurface(_win);
    if (window_surf == nullptr) {
      return EF("Window has no surface to blit to");
    }
    int ret = SDL_BlitScaled(surface, nullptr, window_surf, nullptr);
    if (ret != 0) {
      return bee::Error::fmt("SDL_BlitSurface failed: $", SDL_GetError());
    }

    SDL_UpdateWindowSurface(_win);

    SDL_FreeSurface(surface);
    return bee::ok();
  }

  SDL_Window* _win;
};

} // namespace

Window::~Window() {}

bee::OrError<Window::ptr> Window::create(
  const SDLContext& ctx, const std::string& title, const vec2i& size)
{
  return WindowImpl::create(ctx, title, size);
}

} // namespace sdl
