#include "sdl_context.hpp"

#include "sdl_error.hpp"
#include "sdl_header.hpp"

#include "bee/span.hpp"

using std::nullopt;
using std::optional;

namespace sdl {

namespace {

optional<MouseButton> mouse_button_of_sdl(int sdl_button)
{
  switch (sdl_button) {
  case SDL_BUTTON_LEFT:
    return MouseButton::Left;
  case SDL_BUTTON_MIDDLE:
    return MouseButton::Middle;
  case SDL_BUTTON_RIGHT:
    return MouseButton::Right;
  case SDL_BUTTON_X1:
    return MouseButton::X1;
  case SDL_BUTTON_X2:
    return MouseButton::X2;
  }
  return nullopt;
}

} // namespace

SDLContext::~SDLContext() { SDL_Quit(); }

bee::OrError<SDLContext::ptr> SDLContext::create()
{
  bail_unit_sdl(SDL_Init(SDL_INIT_EVERYTHING));
  return ptr(new SDLContext());
}

SDLContext::SDLContext() {}

bee::OrError<optional<Event>> SDLContext::poll_event(
  const std::optional<bee::Span>& timeout)
{
  bool is_first = true;

  while (true) {
    SDL_Event event;
    if (is_first && timeout.has_value()) {
      if (SDL_WaitEventTimeout(&event, timeout->to_millis()) == 0) {
        return nullopt;
      }
    } else {
      if (SDL_PollEvent(&event) == 0) { return nullopt; }
    }
    is_first = false;
    switch (event.type) {
    case SDL_QUIT:
      return Event::QuitEvent{};
      break;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      return Event::KeyboardEvent{
        .action =
          event.type == SDL_KEYDOWN ? KeyAction::KeyDown : KeyAction::KeyUp,
        .key = key_code_of_sdl_key(event.key.keysym.sym),
        .repeat = event.key.repeat != 0,
      };
      break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP: {
      auto& be = event.button;
      if (auto button = mouse_button_of_sdl(be.button)) {
        return Event::MouseButtonEvent{
          .action = event.type == SDL_MOUSEBUTTONDOWN
                    ? MouseButtonAction::ButtonDown
                    : MouseButtonAction::ButtonUp,
          .button = *button,
          .x = be.x,
          .y = be.y,
        };
      }
    } break;
    case SDL_MOUSEMOTION: {
      auto& me = event.button;
      return Event::MouseMotionEvent{.x = me.x, .y = me.y};
    } break;
    case SDL_MOUSEWHEEL: {
      auto& ev = event.wheel;
      return Event::MouseScrollEvent{.x = ev.x, .y = ev.y};
    } break;
    default:
      break;
    }
  }
}

} // namespace sdl
