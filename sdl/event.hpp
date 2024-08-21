#pragma once

#include <string>
#include <variant>

#include "key_code.hpp"

namespace sdl {

enum class KeyAction {
  KeyUp,
  KeyDown,
};

enum class MouseButtonAction {
  ButtonUp,
  ButtonDown,
};

enum class MouseButton {
  Left,
  Middle,
  Right,
  X1,
  X2,
};

enum class ScrollDirection {
  Up,
  Down,
};

struct EventKind {
 public:
  enum Value {
    Quit,
    Keyboard,
    MouseButton,
    MouseMotion,
    MouseScroll,
  };

  EventKind(Value v) : _v(v) {}

  operator Value() const { return _v; }

  std::string to_string() const
  {
    switch (_v) {
    case Quit:
      return "Quit";
    case Keyboard:
      return "Keyboard";
    case MouseButton:
      return "MouseButton";
    case MouseMotion:
      return "MouseMotion";
    case MouseScroll:
      return "MouseScroll";
    }
  }

 private:
  Value _v;
};

struct Event {
 public:
  struct QuitEvent {
    EventKind kind() const { return EventKind::Quit; }
  };

  struct KeyboardEvent {
    KeyAction action;
    KeyCode key;
    bool repeat;

    EventKind kind() const { return EventKind::Keyboard; }
  };

  struct MouseButtonEvent {
    MouseButtonAction action;
    MouseButton button;
    int x;
    int y;
    EventKind kind() const { return EventKind::MouseButton; }
  };

  struct MouseMotionEvent {
    int x;
    int y;
    EventKind kind() const { return EventKind::MouseMotion; }
  };

  struct MouseScrollEvent {
    int x;
    int y;
    EventKind kind() const { return EventKind::MouseScroll; }
  };

  template <class T>
  Event(T&& v)
      : _v(std::forward<T>(v)),
        _kind(std::visit([](const auto& e) { return e.kind(); }, _v))
  {}

  template <class F> constexpr auto visit(F&& f) const
  {
    return std::visit(f, _v);
  }

  EventKind kind() const { return _kind; }

 private:
  std::variant<
    QuitEvent,
    KeyboardEvent,
    MouseButtonEvent,
    MouseMotionEvent,
    MouseScrollEvent>
    _v;

  EventKind _kind;
};

} // namespace sdl
