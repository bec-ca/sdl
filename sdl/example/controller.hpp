#pragma once

#include <functional>
#include <memory>
#include <variant>

#include "level.hpp"

#include "bee/or_error.hpp"
#include "sdl/event.hpp"
#include "sdl/renderer.hpp"

namespace sdl::example {

struct ControllerStatus {
  struct Continue {};
  struct Exit {};
  struct StartGame {
    std::optional<Level> level;
  };
  struct Back {};
  struct StartLevelEditor {};

  template <class T> ControllerStatus(T&& t) : _v(std::forward<T>(t)) {}

  template <class F> constexpr auto visit(F&& f) const noexcept
  {
    return std::visit(f, _v);
  }

  template <class F> constexpr auto visit(F&& f) noexcept
  {
    return std::visit(f, _v);
  }

 private:
  std::variant<Continue, Exit, StartGame, Back, StartLevelEditor> _v;
};

struct Controller {
 public:
  using ptr = std::unique_ptr<Controller>;

  virtual ~Controller();

  virtual ControllerStatus handle_event(const Event& event) = 0;

  virtual void tick() = 0;

  virtual bee::OrError<> render(Renderer& ren) = 0;
};

} // namespace sdl::example
