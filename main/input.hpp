#ifndef INPUT_H_
#define INPUT_H_

#include <M5Cardputer.h>

#include <vector>

class InputHandler {
  Keyboard_Class::KeysState last_state;
  std::vector<Point2D_t> last_keys;

 public:
  std::vector<char> update_keypresses();
  const Keyboard_Class::KeysState &st() { return last_state; }
};

#endif  // INPUT_H_
