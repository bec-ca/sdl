#pragma once

namespace sdl {

enum class KeyCode {
  Escape,
  Space,
  Enter,
  Left,
  Right,
  Up,
  Down,
  Plus,
  Minus,
  Equal,
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,
  Other,
};

KeyCode key_code_of_sdl_key(int sdl_key);

} // namespace sdl
