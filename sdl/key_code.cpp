#include "key_code.hpp"

#include <unordered_map>

#include "sdl_header.hpp"

using std::unordered_map;

namespace sdl {

namespace {

unordered_map<int, KeyCode> _key_mapping = {
  {SDLK_ESCAPE, KeyCode::Escape},
  {SDLK_LEFT, KeyCode::Left},
  {SDLK_RIGHT, KeyCode::Right},
  {SDLK_UP, KeyCode::Up},
  {SDLK_DOWN, KeyCode::Down},
  {SDLK_SPACE, KeyCode::Space},
  {SDLK_RETURN, KeyCode::Enter},
  {SDLK_PLUS, KeyCode::Plus},
  {SDLK_KP_PLUS, KeyCode::Plus},
  {SDLK_MINUS, KeyCode::Minus},
  {SDLK_KP_MINUS, KeyCode::Minus},
  {SDLK_EQUALS, KeyCode::Equal},
  {SDLK_KP_EQUALS, KeyCode::Equal},
  {SDLK_a, KeyCode::A},
  {SDLK_b, KeyCode::B},
  {SDLK_c, KeyCode::C},
  {SDLK_d, KeyCode::D},
  {SDLK_e, KeyCode::E},
  {SDLK_f, KeyCode::F},
  {SDLK_g, KeyCode::G},
  {SDLK_h, KeyCode::H},
  {SDLK_i, KeyCode::I},
  {SDLK_j, KeyCode::J},
  {SDLK_k, KeyCode::K},
  {SDLK_l, KeyCode::L},
  {SDLK_m, KeyCode::M},
  {SDLK_n, KeyCode::N},
  {SDLK_o, KeyCode::O},
  {SDLK_p, KeyCode::P},
  {SDLK_q, KeyCode::Q},
  {SDLK_r, KeyCode::R},
  {SDLK_s, KeyCode::S},
  {SDLK_t, KeyCode::T},
  {SDLK_u, KeyCode::U},
  {SDLK_v, KeyCode::V},
  {SDLK_w, KeyCode::W},
  {SDLK_x, KeyCode::X},
  {SDLK_y, KeyCode::Y},
  {SDLK_z, KeyCode::Z},
};

} // namespace

KeyCode key_code_of_sdl_key(SDL_Keycode code)
{
  auto it = _key_mapping.find(code);
  if (it != _key_mapping.end()) { return it->second; }
  return KeyCode::Other;
}

} // namespace sdl
