#pragma once

#include "vec2.hpp"

#include "yasf/cof.hpp"

namespace sdl {

template <class T> struct Rect {
  using value_type = vec2<T>;

  value_type pos;
  value_type size;

  template <class U> inline constexpr Rect operator*(U v) noexcept
  {
    return Rect{pos * v, size * v};
  }

  bool intersect(const Rect& other) const
  {
    auto other_right_corner = other.pos + other.size;
    if (pos.y >= other_right_corner.y) { return false; }
    if (pos.x >= other_right_corner.x) { return false; }
    auto right_corner = pos + size;
    if (right_corner.y <= other.pos.y) { return false; }
    if (right_corner.x <= other.pos.x) { return false; }
    return true;
  }

  bool contains(const vec2i& v) const
  {
    auto rcorner = pos + size;
    return v.x >= pos.x && v.y >= pos.y && v.x < rcorner.x && v.y < rcorner.y;
  }

  vec2i min_corner() const { return pos; }

  vec2i max_corner() const { return pos + size; }

  Rect intersection(const Rect& other) const
  {
    auto corner1 = min_corner().max(other.min_corner());
    auto corner2 = max_corner().min(other.max_corner());
    return {corner1, (corner2 - corner1).max({0, 0})};
  }

  Rect translate(const value_type& t) const { return {pos + t, size}; }

  Rect sub(const Rect& inner) const
  {
    auto other = inner.translate(pos);
    return intersection(other);
  }

  static Rect of_corners(const value_type& corner1, const value_type& corner2)
  {
    auto x1 = std::min(corner1.x, corner2.x);
    auto x2 = std::max(corner1.x, corner2.x);
    auto y1 = std::min(corner1.y, corner2.y);
    auto y2 = std::max(corner1.y, corner2.y);

    return {.pos = {x1, y1}, .size = {x2 - x1, y2 - y1}};
  }

  static constexpr Rect zero() { return {0, 0}; }

  auto operator<=>(const Rect& other) const = default;

  // yasf

  using fmt = std::pair<value_type, value_type>;

  yasf::Value::ptr to_yasf_value() const { return yasf::ser(fmt(pos, size)); }

  static bee::OrError<Rect> of_yasf_value(const yasf::Value::ptr value)
  {
    bail(pair, yasf::des<fmt>(value));
    return Rect{pair.first, pair.second};
  }

  // to_string

  std::string to_string() const { return yasf::Cof::serialize(*this); }
};

using Recti = Rect<int>;
using Rectf = Rect<float>;
using Rectd = Rect<double>;

} // namespace sdl
