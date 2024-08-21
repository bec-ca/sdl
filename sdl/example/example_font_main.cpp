#include <variant>

#include "bee/print.hpp"
#include "sdl/font.hpp"
#include "sdl/renderer.hpp"
#include "sdl/sdl_context.hpp"
#include "sdl/window.hpp"

namespace sdl::example {

void main()
{
  must(ctx, SDLContext::create());
  must(win, Window::create(*ctx, "Example Game", {800, 600}));
  must(ren, Renderer::create(*win, {.blend_mode = BlendMode::Add}));

  must_unit(TTF::init());
  must(font, Font::create(48));
  P("Chosen font: $", font->info());
  must(texture, font->render_text(*ren, "Hello world!"));

  bool running = true;
  while (running) {
    must_unit(ren->clear());
    must_unit(
      ren->fill_rect(*texture, (ren->viewport().size - texture->size()) / 2));
    ren->present();

    while (true) {
      must(event, ctx->poll_event());
      if (!event.has_value()) { break; }
      if (event->kind() == EventKind::Quit) { running = false; }
    }
  }
}

} // namespace sdl::example

int main()
{
  sdl::example::main();
  return 0;
}
