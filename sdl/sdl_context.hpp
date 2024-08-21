#pragma once

#include <memory>

#include "event.hpp"

#include "bee/or_error.hpp"
#include "bee/span.hpp"

namespace sdl {

struct SDLContext {
 public:
  using ptr = std::unique_ptr<SDLContext>;
  ~SDLContext();

  SDLContext(const SDLContext& other) = delete;
  SDLContext(SDLContext&& other) = delete;

  static bee::OrError<ptr> create();

  bee::OrError<std::optional<Event>> poll_event(
    const std::optional<bee::Span>& timeout = std::nullopt);

 private:
  SDLContext();
};

} // namespace sdl
