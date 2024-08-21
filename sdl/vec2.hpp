#pragma once

#include <compare>
#include <concepts>

#include "yasf/serializer.hpp"
#include "yasf/value.hpp"

namespace sdl {

template <class T> struct vec2;

namespace details {

struct sum_t {
  template <class T, class U>
  constexpr inline auto operator()(const T& a, const U& b) const noexcept
  {
    return a + b;
  }
};

struct sub_t {
  template <class T, class U>
  constexpr inline auto operator()(const T& a, const U& b) const noexcept
  {
    return a - b;
  }
};

struct mul_t {
  template <class T, class U>
  constexpr inline auto operator()(const T& a, const U& b) const noexcept
  {
    return a * b;
  }
};

struct div_t {
  template <class T, class U>
  constexpr inline auto operator()(const T& a, const U& b) const noexcept
  {
    return a / b;
  }
};

constexpr sum_t sum;
constexpr sub_t sub;
constexpr mul_t mul;
constexpr div_t div;

template <class T> struct is_vec2_t;

template <class T> struct is_vec2_t<vec2<T>> {
  static constexpr bool value = true;
};

template <class T> struct is_vec2_t {
  static constexpr bool value = false;
};

template <class T>
concept is_vec2 = is_vec2_t<T>::value;

template <class T>
concept is_scalar = !is_vec2_t<std::decay_t<T>>::value;

} // namespace details

enum class Dir {
  Up,
  Left,
  Down,
  Right,
};

struct Axis {
 public:
  enum Value {
    X,
    Y,
  };

  constexpr Axis(Value v) : _value(v) {}

  constexpr operator Value() const { return _value; }

  constexpr Value other() const
  {
    switch (_value) {
    case X:
      return Y;
    case Y:
      return X;
    }
  }

 private:
  Value _value;
};

inline constexpr Dir axis_dir(Axis axis, bool positive) noexcept
{
  switch (axis) {
  case Axis::X:
    if (positive) {
      return Dir::Right;
    } else {
      return Dir::Left;
    }
    break;
  case Axis::Y:
    if (positive) {
      return Dir::Down;
    } else {
      return Dir::Up;
    }
    break;
  }
  assert(false);
}

template <class T> struct AxisPair {
 public:
  T x, y;

  constexpr AxisPair() {}

  template <class U, class V>
  constexpr AxisPair(U&& xa, V&& ya)
      : x(std::forward<U>(xa)), y(std::forward<V>(ya))
  {}

  inline constexpr T get(Axis axis) const
  {
    switch (axis) {
    case Axis::X:
      return x;
    case Axis::Y:
      return y;
    }
    assert(false);
  }

  inline constexpr T& get(Axis axis)
  {
    switch (axis) {
    case Axis::X:
      return x;
    case Axis::Y:
      return y;
    }
    assert(false);
  }

  inline constexpr T operator[](Axis axis) const { return get(axis); }

  inline constexpr T& operator[](Axis axis) { return get(axis); }

  auto operator<=>(const AxisPair& other) const = default;
};

template <class T> struct vec2 : public AxisPair<T> {
  // Static creators

  using AxisPair<T>::AxisPair;

  constexpr static vec2 of_first_axis(Axis first_axis, T v1, T v2)
  {
    switch (first_axis) {
    case Axis::X:
      return {v1, v2};
    case Axis::Y:
      return {v2, v1};
    }
  }

  // operators

  auto operator<=>(const vec2& other) const = default;

  // Scalar ops

  template <details::is_scalar U, std::invocable<T, U> F>
  inline constexpr vec2 binary_op_scalar(const U& v, const F& f) const noexcept
  {
    return {T(f(this->x, v)), T(f(this->y, v))};
  }

  template <details::is_scalar U>
  inline constexpr vec2 operator/(U&& v) const noexcept
  {
    return binary_op_scalar(v, details::div);
  }

  template <details::is_scalar U>
  inline constexpr vec2 operator*(U&& v) const noexcept
  {
    return binary_op_scalar(v, details::mul);
  }

  template <details::is_scalar U>
  inline constexpr vec2 operator+(U&& v) const noexcept
  {
    return binary_op_scalar(v, details::sum);
  }

  template <details::is_scalar U>
  inline constexpr vec2 operator-(U&& v) const noexcept
  {
    return binary_op_scalar(v, details::sub);
  }

  inline constexpr T sum() const noexcept { return this->x + this->y; }

  // Vectorial ops

  inline constexpr vec2 min(const vec2& other) const noexcept
  {
    return {std::min<T>(this->x, other.x), std::min<T>(this->y, other.y)};
  }

  inline constexpr vec2 max(const vec2& other) const noexcept
  {
    return {std::max<T>(this->x, other.x), std::max<T>(this->y, other.y)};
  }

  template <class F>
  inline constexpr vec2 binary_op(const vec2& other, const F& f) const noexcept
  {
    return {T(f(this->x, other.x)), T(f(this->y, other.y))};
  }

  template <class F>
  inline constexpr vec2& binary_op_mut(const vec2& other, const F& f) noexcept
  {
    this->x = f(this->x, other.x);
    this->y = f(this->y, other.y);
    return *this;
  }

  inline constexpr vec2 operator+(const vec2& other) const noexcept
  {
    return binary_op(other, details::sum);
  }

  inline constexpr vec2 operator-(const vec2& other) const noexcept
  {
    return binary_op(other, details::sub);
  }

  inline constexpr vec2 operator*(const vec2& other) const noexcept
  {
    return binary_op(other, details::mul);
  }

  inline constexpr vec2 operator/(const vec2& other) const noexcept
  {
    return binary_op(other, details::div);
  }

  inline constexpr vec2& operator+=(const vec2& other) noexcept
  {
    return binary_op_mut(other, details::sum);
  }

  inline constexpr vec2& operator-=(const vec2& other) noexcept
  {
    return binary_op_mut(other, details::sub);
  }

  inline constexpr vec2& operator*=(const vec2& other) noexcept
  {
    return binary_op_mut(other, details::mul);
  }

  inline constexpr vec2& operator/=(const vec2& other) noexcept
  {
    return binary_op_mut(other, details::div);
  }

  template <class U> inline constexpr vec2<U> cast() const noexcept
  {
    return {U(this->x), U(this->y)};
  }

  // constants

  static vec2 zero() { return vec2{.x = 0, .y = 0}; }
  static vec2 one() { return vec2{.x = 1, .y = 1}; }

  // yasf

  using fmt = std::pair<T, T>;

  yasf::Value::ptr to_yasf_value() const
  {
    return yasf::ser(fmt(this->x, this->y));
  }

  static bee::OrError<vec2> of_yasf_value(const yasf::Value::ptr value)
  {
    bail(pair, yasf::des<fmt>(value));
    return vec2{pair.first, pair.second};
  }
};

using vec2i = vec2<int>;
using vec2d = vec2<double>;
using vec2f = vec2<float>;

} // namespace sdl
