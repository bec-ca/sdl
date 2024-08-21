#include "level_editor.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <set>
#include <vector>

#include "constants.hpp"
#include "controller.hpp"
#include "level.hpp"

#include "bee/file_reader.hpp"
#include "bee/file_writer.hpp"
#include "bee/print.hpp"
#include "bee/time.hpp"
#include "sdl/event.hpp"
#include "sdl/key_mapping.hpp"
#include "sdl/rect.hpp"
#include "sdl/texture.hpp"
#include "yasf/cof.hpp"

using std::make_unique;
using std::max;
using std::min;
using std::nullopt;
using std::optional;
using std::set;
using std::string;
using std::vector;

namespace sdl::example {

namespace {

enum class Action {
  Exit,
  ZoomIn,
  ZoomOut,

  MoveViewUp,
  MoveViewLeft,
  MoveViewDown,
  MoveViewRight,

  MoveCursorUp,
  MoveCursorLeft,
  MoveCursorDown,
  MoveCursorRight,

  ToggleBlock,
  AddBlock,
  RemoveBlock,
  StartSelection,
  PlacePlayer,

  PlayLevel,
};

struct KM {
  KM()
  {
    _map.add(Action::Exit, {KeyCode::Escape, KeyCode::Q});
    _map.add(Action::ZoomIn, {KeyCode::Equal});
    _map.add(Action::ZoomOut, {KeyCode::Minus});

    _map.add(Action::MoveViewUp, {KeyCode::W});
    _map.add(Action::MoveViewLeft, {KeyCode::A});
    _map.add(Action::MoveViewDown, {KeyCode::S});
    _map.add(Action::MoveViewRight, {KeyCode::D});

    _map.add(Action::MoveCursorUp, {KeyCode::Up, KeyCode::K});
    _map.add(Action::MoveCursorLeft, {KeyCode::Left, KeyCode::H});
    _map.add(Action::MoveCursorDown, {KeyCode::Down, KeyCode::J});
    _map.add(Action::MoveCursorRight, {KeyCode::Right, KeyCode::L});

    _map.add(Action::ToggleBlock, {KeyCode::G});
    _map.add(Action::AddBlock, {KeyCode::C});
    _map.add(Action::RemoveBlock, {KeyCode::X});
    _map.add(Action::StartSelection, {KeyCode::V});

    _map.add(Action::PlacePlayer, {KeyCode::P});

    _map.add(Action::PlayLevel, {KeyCode::T});
  }

  optional<Action> get_action(KeyCode code) const
  {
    return _map.get_action(code);
  }

 private:
  KeyMapping<Action> _map;
};

Recti selection_box(vec2i corner1, vec2i corner2)
{
  int x0 = min(corner1.x, corner2.x);
  int x1 = max(corner1.x, corner2.x) + 1;
  int y0 = min(corner1.y, corner2.y);
  int y1 = max(corner1.y, corner2.y) + 1;
  return {vec2i{x0, y0}, vec2i{x1 - x0, y1 - y0}};
}

static constexpr int block_size = 64;

Level create_level(const set<vec2i>& blocks, vec2i player_initial_pos)
{
  vector<Recti> block_rects;

  for (const auto& b : blocks) {
    block_rects.push_back({{b * block_size}, {block_size, block_size}});
  }

  return Level{
    .player_initial_pos = player_initial_pos,
    .blocks = std::move(block_rects),
  };
}

const bee::FilePath level_filename("level.cof");

bee::OrError<Level> load_level()
{
  return yasf::Cof::deserialize_file<Level>(level_filename);
}

bee::OrError<> save_level(const Level& level)
{
  return yasf::Cof::serialize_file(level_filename, level);
}

struct LevelEditorController : Controller {
 public:
  static constexpr double zoom_speed = 1.02;
  static constexpr double scroll_zoom_speed = 1.1;

  virtual void tick() override {}

  vec2d project(const vec2d& v) const { return v * _zoom - _view_offset; }

  vec2d unproject(const vec2d& v) const { return (v + _view_offset) / _zoom; }

  void apply_zoom(double zoom_delta, vec2d pivot)
  {
    auto center = unproject(pivot);
    _zoom *= zoom_delta;
    _view_offset = center * _zoom - pivot;
  }

  virtual bee::OrError<> render(Renderer& ren) override
  {
    if (_block_texture == nullptr) {
      bail_assign(_block_texture, ren.create_texture(Images::Squares));
    }

    auto viewport = ren.viewport().size.cast<double>();

    double zoom_delta = 1.0;
    if (_is_zooming_in) { zoom_delta *= zoom_speed; }
    if (_is_zooming_out) { zoom_delta /= zoom_speed; }
    if (zoom_delta != 1.0f) {
      auto vp_mid = viewport / 2.0;
      apply_zoom(zoom_delta, vp_mid);
    }
    ren.set_zoom(_zoom);

    const double move_speed = 10.0;
    vec2d move_delta{0, 0};
    if (_moving_down) { move_delta.y += 1; }
    if (_moving_up) { move_delta.y -= 1; }
    if (_moving_right) { move_delta.x += 1; }
    if (_moving_left) { move_delta.x -= 1; }
    _view_offset = _view_offset + move_delta * move_speed;

    if (_mouse.has_value()) {
      auto wc = unproject(_mouse->cast<double>());
      auto cell = wc / block_size;
      _cursor = vec2i{int(floor(cell.x)), int(floor(cell.y))};
    }

    ren.set_view(_view_offset.cast<float>());

    for (const auto& block : _blocks) {
      bail_unit(ren.fill_rect(
        *_block_texture, {block * block_size, {block_size, block_size}}));
    }

    if (_player.has_value()) {
      const Color color = {.r = 255, .g = 255, .b = 255, .a = 255};
      Recti rect{.pos = *_player, .size = Constants::player_size};
      bail_unit(ren.fill_rect(color, rect));
    }

    if (_selection_start.has_value()) {
      const Color selection_color{255, 255, 255, 150};
      const Rect rect = selection_box(_cursor, *_selection_start) * block_size;
      bail_unit(ren.fill_rect(selection_color, rect));
    } else {
      const Color cursor_color{255, 255, 255, 100};
      const Recti cursor_rect = {
        _cursor * block_size, {block_size, block_size}};
      bail_unit(ren.fill_rect(cursor_color, cursor_rect));
    }

    return bee::ok();
  }

  static Controller::ptr create()
  {
    return make_unique<LevelEditorController>(load_level().to_optional());
  }

  virtual ControllerStatus handle_event(const Event& event) override
  {
    return event.visit([this](auto&& e) { return _handle_event(e); });
  }

  ControllerStatus _handle_event(const Event::QuitEvent&)
  {
    return ControllerStatus::Exit{};
  }

  ControllerStatus _handle_event(const Event::MouseButtonEvent& event)
  {
    if (_mouse.has_value()) {
      switch (event.button) {
      case MouseButton::Left:
        switch (event.action) {
        case MouseButtonAction::ButtonDown:
          _selection_start = _cursor;
          break;
        case MouseButtonAction::ButtonUp:
          _handle_add_block();
          break;
        }
        break;
      case MouseButton::Right:
        switch (event.action) {
        case MouseButtonAction::ButtonDown:
          _selection_start = _cursor;
          break;
        case MouseButtonAction::ButtonUp:
          _handle_remove_block();
          break;
        }
        break;
      default:
        break;
      }
    }
    return ControllerStatus::Continue{};
  }

  ControllerStatus _handle_event(const Event::MouseMotionEvent& event)
  {
    _mouse = vec2i{event.x, event.y};
    return ControllerStatus::Continue{};
  }

  ControllerStatus _handle_event(const Event::MouseScrollEvent& event)
  {
    if (_mouse.has_value()) {
      if (event.y > 0) {
        apply_zoom(scroll_zoom_speed, _mouse->cast<double>());
      } else if (event.y < 0) {
        apply_zoom(1.0 / scroll_zoom_speed, _mouse->cast<double>());
      }
    }

    return ControllerStatus::Continue{};
  }

  template <class F> void for_each_in_rect(const Recti& r, F&& f)
  {
    for (int x = 0; x < r.size.x; x++) {
      for (int y = 0; y < r.size.y; y++) { f(vec2i{r.pos.x + x, r.pos.y + y}); }
    }
  }

  Recti get_selection()
  {
    if (!_selection_start.has_value()) {
      return {_cursor, {1, 1}};
    } else {
      return selection_box(_cursor, *_selection_start);
    }
  }

  template <class F> void for_each_in_selection(F&& f)
  {
    for_each_in_rect(get_selection(), std::forward<F>(f));
    _selection_start = nullopt;
  }

  void _handle_toggle_block()
  {
    auto toggle = [this](const vec2i& v) {
      auto it = _blocks.find(v);
      if (it == _blocks.end()) {
        _blocks.insert(v);
      } else {
        _blocks.erase(it);
      }
    };
    for_each_in_selection(toggle);
  }

  void _handle_add_block()
  {
    for_each_in_selection([this](auto&& v) { _blocks.insert(v); });
  }

  void _handle_remove_block()
  {
    for_each_in_selection([this](auto&& v) { _blocks.erase(v); });
  }

  ControllerStatus _handle_play_level()
  {
    if (!_player.has_value()) {
      PE("Player not set");
      return ControllerStatus::Continue{};
    }

    return ControllerStatus::StartGame{
      .level = create_level(_blocks, *_player),
    };
  }

  void _maybe_save_level()
  {
    if (_player.has_value()) {
      auto level = create_level(_blocks, *_player);
      auto res = save_level(level);
      if (res.is_error()) { PE("Failed to save level: $", res.error()); }
    }
  }

  void _handle_move_cursor_keyboard(const vec2i& dir)
  {
    _cursor = _cursor + dir;
    _mouse = nullopt;
  }

  ControllerStatus _handle_event(const Event::KeyboardEvent& event)
  {
    bool activated = event.action == KeyAction::KeyDown;

    auto action = _key_mapping.get_action(event.key);
    if (!action.has_value()) { return ControllerStatus::Continue{}; }

    switch (*action) {
    case Action::Exit:
      if (activated) {
        _maybe_save_level();
        return ControllerStatus::Back{};
      }
      break;
    case Action::ZoomIn:
      _is_zooming_in = activated;
      break;
    case Action::ZoomOut:
      _is_zooming_out = activated;
      break;
    case Action::MoveViewRight:
      _moving_right = activated;
      break;
    case Action::MoveViewLeft:
      _moving_left = activated;
      break;
    case Action::MoveViewDown:
      _moving_down = activated;
      break;
    case Action::MoveViewUp:
      _moving_up = activated;
      break;

    case Action::MoveCursorUp:
      if (activated) _handle_move_cursor_keyboard({0, -1});
      break;
    case Action::MoveCursorDown:
      if (activated) _handle_move_cursor_keyboard({0, 1});
      break;
    case Action::MoveCursorLeft:
      if (activated) _handle_move_cursor_keyboard({-1, 0});
      break;
    case Action::MoveCursorRight:
      if (activated) _handle_move_cursor_keyboard({1, 0});
      break;

    case Action::ToggleBlock:
      if (activated) { _handle_toggle_block(); }
      break;
    case Action::AddBlock:
      if (activated) { _handle_add_block(); }
      break;
    case Action::RemoveBlock:
      if (activated) { _handle_remove_block(); }
      break;
    case Action::StartSelection:
      if (activated) { _selection_start = _cursor; }
      break;

    case Action::PlacePlayer:
      if (activated) { _player = _cursor * block_size; }
      break;

    case Action::PlayLevel:
      if (activated) {
        _maybe_save_level();
        return _handle_play_level();
      }
      break;
    }

    return ControllerStatus::Continue{};
  }

  LevelEditorController(std::optional<Level>&& level)
  {
    if (level.has_value()) {
      _player = level->player_initial_pos;
      for (const auto& block : level->blocks) {
        _blocks.insert(block.pos / block_size);
      }
    }
  }

 private:
  KM _key_mapping;

  vec2i _cursor = {0, 0};

  vec2d _view_offset = {0, 0};
  double _zoom = 1.0;

  bool _moving_up = false;
  bool _moving_left = false;
  bool _moving_down = false;
  bool _moving_right = false;

  bool _is_zooming_in = false;
  bool _is_zooming_out = false;

  set<vec2i> _blocks;

  optional<vec2i> _selection_start;

  optional<vec2i> _player;

  Texture::ptr _block_texture;

  optional<vec2i> _mouse;
};

} // namespace

Controller::ptr LevelEditor::create()
{
  return LevelEditorController::create();
}

} // namespace sdl::example
