#ifndef POPUP_H_
#define POPUP_H_

#include <M5Cardputer.h>

class Popup {
  // createSprite and pushSprite are handled in draw()
  virtual void draw_virt(m5gfx::M5Canvas &disp) = 0;
  virtual bool handle_input_virt(char c,
                                 const Keyboard_Class::KeysState &state) = 0;

 public:
  virtual ~Popup() = 0;

  void draw(m5gfx::M5Canvas &disp);
  // returns false if we want to quit
  bool handle_input(char c, const Keyboard_Class::KeysState &state) {
    return handle_input_virt(c, state);
  }
};

class HelpPopup : public Popup {
  void draw_virt(m5gfx::M5Canvas &disp) override;
  bool handle_input_virt(char c,
                         const Keyboard_Class::KeysState &state) override;

 public:
  ~HelpPopup() override;
};

#endif  // POPUP_H_
