#pragma once

#include <optional>
#include <unordered_map>

#include "key_code.hpp"

namespace sdl {

template <class T> struct KeyMapping {
 public:
  void add(T action, const std::initializer_list<KeyCode>& keys)
  {
    for (auto k : keys) { _mappings.emplace(k, action); }
  }

  KeyMapping() {}

  std::optional<T> get_action(KeyCode code) const
  {
    auto it = _mappings.find(code);
    if (it == _mappings.end()) { return std::nullopt; }
    return it->second;
  }

 private:
  std::unordered_map<KeyCode, T> _mappings;
};

} // namespace sdl
