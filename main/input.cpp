#include "input.hpp"

#include <algorithm>

std::vector<char> InputHandler::update_keypresses() {
  if (!M5Cardputer.Keyboard.isChange()) {
    return std::vector<char>();
  }
  auto cmp = [](const Point2D_t& lhs, const Point2D_t& rhs) {
    return lhs.x < rhs.x ? true : lhs.y < rhs.y;
  };

  std::vector<Point2D_t> new_keys = M5Cardputer.Keyboard.keyList();
  std::sort(new_keys.begin(), new_keys.end(), cmp);
  std::vector<Point2D_t> keydowns;
  std::set_difference(new_keys.begin(), new_keys.end(), last_keys.begin(),
                      last_keys.end(), std::back_inserter(keydowns), cmp);
  last_keys = new_keys;

  std::vector<char> keys;
  for (auto key : keydowns) {
    // We have to check the first value, since the second value can
    // conflict with actual keypresses (see `_key_value_map`)
    // This is hacky, but it's what `Keyboard_Class::updateKeysState` does.
    // Probably KEY_ENTER should be 10 (or whatever) instead of 0x28 = '('
    // same with KEY_BACKSPACE = '*' and KEY_TAB = '+'
    char base_val = M5Cardputer.Keyboard.getKeyValue(key).value_first;
    switch (base_val) {
      case KEY_FN:
      case KEY_OPT:
      case KEY_LEFT_CTRL:
      case KEY_LEFT_SHIFT:
      case KEY_LEFT_ALT:
      case KEY_TAB:
      case KEY_BACKSPACE:
      case KEY_ENTER:
        break;
      default:
        keys.push_back(M5Cardputer.Keyboard.getKey(key));
    }
  }

  Keyboard_Class::KeysState new_state = M5Cardputer.Keyboard.keysState();
  if (new_state.enter && !last_state.enter) {
    keys.push_back('\n');
  }
  if (new_state.tab && !last_state.tab) {
    keys.push_back('\t');
  }
  last_state = new_state;
  return keys;
}
