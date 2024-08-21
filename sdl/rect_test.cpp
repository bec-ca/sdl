#include "rect.hpp"

#include "bee/testing.hpp"

namespace sdl {
namespace {

TEST(sub)
{
  auto run = [](const Recti r1, const Recti r2) {
    P("sub($, $) -> $", r1, r2, r1.sub(r2));
  };
  run({{10, 10}, {100, 100}}, {{10, 10}, {10, 10}});
  run({{0, 0}, {100, 100}}, {{10, 10}, {10, 10}});
  run({{0, 0}, {100, 100}}, {{10, 10}, {100, 100}});
  run({{10, 10}, {100, 100}}, {{10, 10}, {100, 100}});
  run({{10, 10}, {100, 100}}, {{200, 200}, {100, 100}});
  run({{10, 10}, {100, 100}}, {{-10, -10}, {100, 100}});
}

} // namespace
} // namespace sdl
