#include "font.hpp"

#include "bee/testing.hpp"

namespace sdl {
namespace {

TEST(init)
{
  P(TTF::init());
  must(font, Font::create(24));
  P(font->info().style);
}

} // namespace

} // namespace sdl
