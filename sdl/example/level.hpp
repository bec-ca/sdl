#pragma once

#include <vector>

#include "sdl/rect.hpp"
#include "sdl/vec2.hpp"

namespace sdl::example {

struct Level {
 public:
  vec2i player_initial_pos;
  std::vector<Recti> blocks;

  using fmt = std::pair<vec2i, std::vector<Recti>>;

  yasf::Value::ptr to_yasf_value() const
  {
    return yasf::ser(fmt(player_initial_pos, blocks));
  }

  static bee::OrError<Level> of_yasf_value(const yasf::Value::ptr value)
  {
    bail(pair, yasf::des<fmt>(value));
    return Level{
      .player_initial_pos = pair.first,
      .blocks = std::move(pair.second),
    };
  }
};

}; // namespace sdl::example
