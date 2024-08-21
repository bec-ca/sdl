#include <memory>

#include "controller.hpp"
#include "in_game.hpp"
#include "level_editor.hpp"
#include "menu.hpp"

#include "bee/or_error.hpp"
#include "bee/print.hpp"
#include "sdl/renderer.hpp"
#include "sdl/sdl_context.hpp"
#include "sdl/window.hpp"

using std::make_unique;
using std::unique_ptr;
using std::vector;

namespace sdl::example {

namespace {

struct Main {
 public:
  using ptr = std::unique_ptr<Main>;

  static bee::OrError<ptr> create()
  {
    bail(ctx, SDLContext::create());
    bail(win, Window::create(*ctx, "Example Game", {2400, 1800}));
    bail(ren, Renderer::create(*win, {.blend_mode = BlendMode::Add}));
    return make_unique<Main>(std::move(ctx), std::move(win), std::move(ren));
  }

  void _handle_status(const ControllerStatus::Exit&) { _running = false; }
  void _handle_status(const ControllerStatus::Continue&) {}

  void _handle_status(ControllerStatus::StartGame& event)
  {
    push_controller(InGame::create(std::move(event.level)));
  }

  void _handle_status(const ControllerStatus::Back&) { pop_controller(); }

  void _handle_status(const ControllerStatus::StartLevelEditor&)
  {
    push_controller(LevelEditor::create());
  }

  void push_controller(Controller::ptr&& controller)
  {
    if (_controller != nullptr) { _stack.push_back(std::move(_controller)); }
    _controller = std::move(controller);
  }

  void pop_controller()
  {
    assert(!_stack.empty());
    _controller = std::move(_stack.back());
    _stack.pop_back();
  }

  bee::OrError<> render()
  {
    bail_unit(_ren->clear());
    bail_unit(_controller->render(*_ren));
    _ren->present();
    return bee::ok();
  }

  void tick() { _controller->tick(); }

  bee::OrError<> main_loop()
  {
    while (_running) {
      vector<ControllerStatus> queue;
      while (true) {
        bail(event, _ctx->poll_event());
        if (!event.has_value()) { break; }
        auto result = _controller->handle_event(*event);
        queue.push_back(std::move(result));
      }

      for (auto& event : queue) {
        event.visit([this](auto&& result) { _handle_status(result); });
      }

      tick();
      bail_unit(render());
    }

    return bee::ok();
  }

  Main(SDLContext::ptr&& ctx, Window::ptr&& win, Renderer::ptr&& ren)
      : _ctx(std::move(ctx)),
        _win(std::move(win)),
        _ren(std::move(ren)),
        _controller(Menu::create())
  {}

 private:
  SDLContext::ptr _ctx;
  Window::ptr _win;
  Renderer::ptr _ren;

  bool _running = true;

  Controller::ptr _controller;

  vector<Controller::ptr> _stack;
};

} // namespace

bee::OrError<> run()
{
  bail(main, Main::create());
  return main->main_loop();
}

} // namespace sdl::example

int main()
{
  auto ret = sdl::example::run();
  if (ret.is_error()) {
    PE(ret.error());
    return EXIT_FAILURE;
  }
  return 0;
}
