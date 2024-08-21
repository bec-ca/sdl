#include "menu.hpp"

#include <algorithm>
#include <vector>

#include "sdl/key_mapping.hpp"
#include "sdl/text_writer.hpp"

using std::clamp;
using std::make_unique;
using std::optional;
using std::string;
using std::vector;

namespace sdl::example {
namespace {

enum class MenuAction {
  Exit,
  LevelEditor,
  Start,
};

struct MenuItem {
 public:
  MenuItem(const string& label, MenuAction action)
      : _label(label), _action(action)
  {}

  MenuAction action() const { return _action; }

  const std::string& label() const { return _label; }

  const Recti& rect() const { return _rect; }
  Recti& rect() { return _rect; }

 private:
  string _label;
  MenuAction _action;
  Recti _rect;
};

enum class InputAction {
  MoveUp,
  MoveDown,
  Select,
  Exit,
};

struct MenuKeyMapping {
 public:
  MenuKeyMapping()
  {
    _map.add(InputAction::MoveUp, {KeyCode::Up, KeyCode::W, KeyCode::K});
    _map.add(InputAction::MoveDown, {KeyCode::Down, KeyCode::S, KeyCode::J});
    _map.add(InputAction::Select, {KeyCode::Enter, KeyCode::O});
    _map.add(InputAction::Exit, {KeyCode::Escape, KeyCode::Q});
  }

  optional<InputAction> get_action(KeyCode code) const
  {
    return _map.get_action(code);
  }

 private:
  KeyMapping<InputAction> _map;
};

struct MenuImpl : public Controller {
 public:
  static constexpr int menu_width = 400;
  static constexpr int text_height = 32;
  static constexpr int margin = 4;
  static constexpr int item_height = text_height + margin * 2;

  MenuImpl()
      : _menu_items({
          MenuItem("Start", MenuAction::Start),
          MenuItem("Level Editor", MenuAction::LevelEditor),
          MenuItem("Exit", MenuAction::Exit),
        })
  {}
  virtual ~MenuImpl() {}

  ControllerStatus _handle_event(const Event::QuitEvent&)
  {
    return ControllerStatus::Exit{};
  }

  ControllerStatus _handle_event(const Event::MouseButtonEvent& event)
  {
    if (event.action == MouseButtonAction::ButtonDown) {
      vec2i pos = {event.x, event.y};
      for (const auto& item : _menu_items) {
        if (item.rect().contains(pos)) {
          return _handle_menu_action(item.action());
        }
      }
    }
    return ControllerStatus::Continue{};
  }

  ControllerStatus _handle_event(const Event::MouseMotionEvent& event)
  {
    vec2i pos = {event.x, event.y};
    for (int i = 0; i < std::ssize(_menu_items); i++) {
      const auto& item = _menu_items.at(i);
      if (item.rect().contains(pos)) {
        _selected_menu_item = i;
        break;
      }
    }
    return ControllerStatus::Continue{};
  }

  ControllerStatus _handle_event(const Event::MouseScrollEvent&)
  {
    return ControllerStatus::Continue{};
  }

  virtual ControllerStatus handle_event(const Event& event)
  {
    return event.visit([this](auto&& e) { return _handle_event(e); });
  }

  void move_cursor(int dir)
  {
    _selected_menu_item =
      clamp(_selected_menu_item + dir, 0, int(_menu_items.size()) - 1);
  }

  ControllerStatus _handle_menu_action(MenuAction action)
  {
    switch (action) {
    case MenuAction::Exit:
      return ControllerStatus::Exit{};
    case MenuAction::Start:
      return ControllerStatus::StartGame{};
    case MenuAction::LevelEditor:
      return ControllerStatus::StartLevelEditor{};
    }
    return ControllerStatus::Continue{};
  }

  ControllerStatus _handle_select()
  {
    auto action = _menu_items.at(_selected_menu_item).action();
    return _handle_menu_action(action);
  }

  ControllerStatus _handle_event(const Event::KeyboardEvent& ev)
  {
    if (ev.repeat) { return ControllerStatus::Continue{}; }

    if (ev.action != KeyAction::KeyDown) {
      return ControllerStatus::Continue{};
    }

    auto action = _key_mapping.get_action(ev.key);
    if (!action.has_value()) { return ControllerStatus::Continue{}; }

    switch (*action) {
    case InputAction::Exit:
      return ControllerStatus::Exit{};
    case InputAction::MoveUp:
      move_cursor(-1);
      break;
    case InputAction::MoveDown:
      move_cursor(1);
      break;
    case InputAction::Select:
      return _handle_select();
    }

    return ControllerStatus::Continue{};
  }

  virtual void tick() {}

  void reflow(Renderer& ren)
  {
    vec2i menu_size{menu_width, int(_menu_items.size() * item_height)};

    auto viewport = ren.viewport();

    _menu_location = (viewport.size - menu_size) / 2;

    vec2i item_location = _menu_location;
    for (int i = 0; i < std::ssize(_menu_items); i++) {
      auto& item = _menu_items.at(i);
      item.rect() = Recti{item_location, {menu_width, item_height}};
      item_location.y += item_height;
    }
  }

  virtual bee::OrError<> render(Renderer& ren)
  {
    if (_text_writer == nullptr) {
      bail_assign(_text_writer, TextWriter::create(ren));
    }

    ren.set_view({0, 0});
    ren.set_zoom(1.0);

    reflow(ren);

    for (int i = 0; i < int(_menu_items.size()); i++) {
      auto& item = _menu_items.at(i);
      if (i == _selected_menu_item) {
        bail_unit(ren.fill_rect(Color{30, 30, 30, 255}, item.rect()));
      }
      bail_unit(
        _text_writer->draw_text(ren, item.rect().pos + margin, item.label()));
    }

    return bee::ok();
  }

 private:
  TextWriter::ptr _text_writer;

  vector<MenuItem> _menu_items;

  MenuKeyMapping _key_mapping;

  int _selected_menu_item = 0;

  vec2i _menu_location;
};

} // namespace

Controller::ptr Menu::create() { return make_unique<MenuImpl>(); }

} // namespace sdl::example
