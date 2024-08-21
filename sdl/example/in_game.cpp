#include "in_game.hpp"

#include <algorithm>
#include <optional>
#include <vector>

#include "constants.hpp"
#include "controller.hpp"

#include "bee/time.hpp"
#include "sdl/event.hpp"
#include "sdl/key_mapping.hpp"
#include "sdl/rect.hpp"
#include "sdl/texture.hpp"

using bee::Time;
using std::clamp;
using std::make_unique;
using std::max;
using std::min;
using std::optional;
using std::variant;
using std::vector;

namespace sdl::example {

namespace {

enum class PlayerAction {
  MoveUp,
  MoveDown,
  MoveLeft,
  MoveRight,
  Jump,
};

enum class GeneralAction {
  Exit,
};

struct Action {
 public:
  Action(const Action& other) = default;

  template <class T>
    requires(
      std::is_same_v<std::decay_t<T>, GeneralAction> ||
      std::is_same_v<std::decay_t<T>, PlayerAction>)
  Action(T&& v) : _value(v)
  {}

  template <class F> constexpr auto visit(F&& f) const noexcept
  {
    return std::visit(f, _value);
  }

 private:
  using value_type = variant<GeneralAction, PlayerAction>;

  value_type _value;
};

struct InGameKeyMapping {
  InGameKeyMapping()
  {
    _map.add(Action(GeneralAction::Exit), {KeyCode::Escape, KeyCode::Q});
    _map.add(Action(PlayerAction::MoveUp), {KeyCode::Up, KeyCode::W});
    _map.add(Action(PlayerAction::MoveLeft), {KeyCode::Left, KeyCode::A});
    _map.add(Action(PlayerAction::MoveDown), {KeyCode::Down, KeyCode::S});
    _map.add(Action(PlayerAction::MoveRight), {KeyCode::Right, KeyCode::D});
    _map.add(Action(PlayerAction::Jump), {KeyCode::Space});
  }

  optional<Action> get_action(KeyCode code) const
  {
    return _map.get_action(code);
  }

 private:
  KeyMapping<Action> _map;
};

struct LevelController {
 public:
  LevelController(vector<Recti>&& blocks) : _blocks(std::move(blocks)) {}

  ~LevelController() {}

  const vector<Recti> blocks() const { return _blocks; }

  optional<Dir> move_rect(
    Axis axis, double& speed, vec2d& pos, const vec2i& rect_size) const
  {
    vec2d target = pos;
    target.get(axis) += speed;

    Recti initial_rect{pos.cast<int>(), rect_size};

    optional<Dir> intersect_dir;
    for (const auto& block : _blocks) {
      // Was already intersecting, don't know what to do
      if (block.intersect(initial_rect)) { continue; }
      if (block.intersect({target.cast<int>(), rect_size})) {
        if (speed > 0) {
          target.get(axis) = block.pos.get(axis) - rect_size.get(axis);
          intersect_dir = axis_dir(axis, true);
        } else {
          target.get(axis) = block.pos.get(axis) + block.size.get(axis);
          intersect_dir = axis_dir(axis, false);
        }
      }
    }

    if (intersect_dir.has_value()) { speed = 0; }

    pos = target;

    return intersect_dir;
  };

 private:
  vector<Recti> _blocks;
};

struct JumpController {
 public:
  static constexpr int jump_ticks = 20;

  void start_jumping()
  {
    _is_jumping = true;
    _started_jumping = false;
  }

  void stop_jumping() { _is_jumping = false; }

  bool is_accelerating(bool touching_ground)
  {
    if (!_is_jumping) { return false; }
    if (!_started_jumping) {
      if (touching_ground) {
        _started_jumping = true;
        _remaining_jump_ticks = jump_ticks;
      } else {
        stop_jumping();
        return false;
      }
    }
    if (_remaining_jump_ticks == 0) {
      stop_jumping();
      return false;
    }
    --_remaining_jump_ticks;
    return true;
  }

 private:
  bool _is_jumping = false;
  bool _started_jumping = false;
  int _remaining_jump_ticks = 0;
};

struct PlayerController {
 public:
  PlayerController(const vec2d& pos) : _player_pos(pos) {}

  void set_jumping(bool is_jumping)
  {
    if (is_jumping) {
      _jump_controller.start_jumping();
    } else {
      _jump_controller.stop_jumping();
    }
  }

  void set_moving_right(bool moving_right) { _moving_right = moving_right; }

  void set_moving_left(bool moving_left) { _moving_left = moving_left; }

  void tick(const LevelController& level)
  {
    // vertical movement
    {
      const double max_gravity_speed = 30.0;
      const double gravity_accel = 2.0;
      const double jump_speed = -30;

      if (_jump_controller.is_accelerating(_touching_ground)) {
        _y_speed = jump_speed;
      }

      _y_speed = min(max_gravity_speed, _y_speed + gravity_accel);

      auto dir =
        level.move_rect(Axis::Y, _y_speed, _player_pos, Constants::player_size);

      _touching_ground = (dir.has_value() && *dir == Dir::Down);

      if (dir.has_value()) { _jump_controller.stop_jumping(); }
    }

    // horizontal movement
    {
      const double move_accel = 2.0;
      const double max_move_speed = 10;

      if (_moving_left) { _x_speed -= move_accel; }
      if (_moving_right) { _x_speed += move_accel; }

      if (!_moving_left && !_moving_right) {
        if (_x_speed > 0) {
          _x_speed = max(0.0, _x_speed - move_accel);
        } else if (_x_speed < 0) {
          _x_speed = min(0.0, _x_speed + move_accel);
        }
      }

      _x_speed = clamp(_x_speed, -max_move_speed, max_move_speed);

      level.move_rect(Axis::X, _x_speed, _player_pos, Constants::player_size);
    }
  }

  void handle_event(PlayerAction action, bool activated)
  {
    switch (action) {
    case PlayerAction::MoveUp:
      break;
    case PlayerAction::MoveLeft:
      set_moving_left(activated);
      break;
    case PlayerAction::MoveDown:
      break;
    case PlayerAction::MoveRight:
      set_moving_right(activated);
      break;
    case PlayerAction::Jump:
      set_jumping(activated);
      break;
    }
  }

  Recti rect() const
  {
    return {.pos = _player_pos.cast<int>(), .size = Constants::player_size};
  }

 private:
  JumpController _jump_controller;

  vec2d _player_pos;

  bool _moving_left = false;
  bool _moving_right = false;

  double _x_speed = 0;
  double _y_speed = 0;

  bool _touching_ground = false;
};

struct ViewController {
 public:
  ViewController() {}
  ~ViewController() {}

  void update(const Recti& player_rect, Renderer& ren)
  {
    const int border = 200;

    auto viewport = ren.viewport();
    auto& window_size = viewport.size;
    auto& player_pos = player_rect.pos;

    if (player_pos.x - _view_offset.x >= window_size.x - border) {
      _view_offset.x = player_pos.x - (window_size.x - border);
    }
    if (player_pos.x - _view_offset.x <= border) {
      _view_offset.x = player_pos.x - border;
    }

    if (player_pos.y - _view_offset.y >= window_size.y - border) {
      _view_offset.y = player_pos.y - (window_size.y - border);
    }
    if (player_pos.y - _view_offset.y <= border) {
      _view_offset.y = player_pos.y - border;
    }

    ren.set_view(_view_offset);
    ren.set_zoom(1.0);
  }

 private:
  vec2f _view_offset = {0, 0};
};

struct InGameController : Controller {
 public:
  using ptr = std::unique_ptr<InGameController>;

  virtual void tick() override { _player_controller.tick(_level); }

  virtual bee::OrError<> render(Renderer& ren) override
  {
    if (_block_texture == nullptr) {
      bail_assign(_block_texture, ren.create_texture(Images::Squares));
    }

    auto player_rect = _player_controller.rect();

    _view_controller.update(player_rect, ren);

    const Color player_color = {.r = 255, .g = 255, .b = 255, .a = 255};
    bail_unit(ren.fill_rect(player_color, player_rect));

    // Color block_color = {.r = 255, .g = 50, .b = 50, .a = 255};
    for (const auto& block : _level.blocks()) {
      // bail_unit(ren.fill_rect(block_color, block));
      bail_unit(ren.fill_rect(*_block_texture, block));
    }

    return bee::ok();
  }

  static ptr create(optional<Level>&& level)
  {
    return make_unique<InGameController>(std::move(level));
  }

  virtual ControllerStatus handle_event(const Event& event) override
  {
    return event.visit([this](auto&& e) { return _handle_event(e); });
  }

  ControllerStatus _handle_event(const Event::QuitEvent&)
  {
    return ControllerStatus::Exit{};
  }

  ControllerStatus _handle_event(const Event::KeyboardEvent& event)
  {
    if (event.repeat) { return ControllerStatus::Continue{}; }

    bool activated = event.action == KeyAction::KeyDown;
    auto action = _key_mapping.get_action(event.key);

    if (!action.has_value()) { return ControllerStatus::Continue{}; }

    return action->visit([&]<class T>(const T& action) -> ControllerStatus {
      if constexpr (std::is_same_v<T, PlayerAction>) {
        _player_controller.handle_event(action, activated);
        return ControllerStatus::Continue{};
      } else if constexpr (std::is_same_v<T, GeneralAction>) {
        switch (action) {
        case GeneralAction::Exit:
          return ControllerStatus::Back{};
        }
      }
    });
  }

  ControllerStatus _handle_event(const Event::MouseButtonEvent&)
  {
    return ControllerStatus::Continue{};
  }

  ControllerStatus _handle_event(const Event::MouseMotionEvent&)
  {
    return ControllerStatus::Continue{};
  }

  ControllerStatus _handle_event(const Event::MouseScrollEvent&)
  {
    return ControllerStatus::Continue{};
  }

  static LevelController make_level_controller(optional<Level>& level)
  {
    if (level.has_value()) {
      return LevelController(std::move(level->blocks));
    } else {
      return LevelController(
        {{
           .pos = {0, 800},
           .size = {800 * 2, 800 * 2},
         },
         {
           .pos = {1200, 500},
           .size = {100, 100},
         },
         {
           .pos = {300, 700},
           .size = {100, 100},
         }});
    }
  }

  static vec2d get_player_initial_position(optional<Level>& level)
  {
    if (level.has_value()) {
      return level->player_initial_pos.cast<double>();
    } else {
      return {0, 0};
    }
  }

  InGameController(optional<Level>&& level)
      : _start_time(Time::monotonic()),
        _player_controller(get_player_initial_position(level)),
        _level(make_level_controller(level))
  {}

 private:
  Time _start_time;

  PlayerController _player_controller;

  ViewController _view_controller;

  InGameKeyMapping _key_mapping;

  LevelController _level;

  Texture::ptr _block_texture;
};

} // namespace

Controller::ptr InGame::create(optional<Level>&& level)
{
  return InGameController::create(std::move(level));
}

} // namespace sdl::example
