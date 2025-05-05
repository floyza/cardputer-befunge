#include "popup.hpp"

#include "common.hpp"

Popup::~Popup() {}

void Popup::draw(m5gfx::M5Canvas &disp) {
  disp.createSprite(swidth - 20, sheight - 20);
  draw_virt(disp);
  disp.pushSprite(10, 10);
}

HelpPopup::~HelpPopup() {}

void HelpPopup::draw_virt(m5gfx::M5Canvas &disp) {
  disp.setTextColor(BLACK, LIGHTGREY);
  disp.clear(LIGHTGREY);
  disp.drawString("0-9 +-*/%!><^v?\":#@", 0, 0);
  disp.drawString("_ Pop move right if value=0, left otherwise", 0, 10);
  disp.drawString("| Pop move down if value=0, up otherwise", 0, 20);
  disp.drawString("\\ Swap two top stack", 0, 30);
  disp.drawString("$ Pop stack, discard", 0, 40);
  disp.drawString("p Pop y, x, v, (x,y) to v", 0, 50);
  disp.drawString("g Pop y and x, get", 0, 60);
  disp.drawString("` Pop b and a, 1 if a>b else 0", 0, 60);
}

bool HelpPopup::handle_input_virt(char c,
                                  const Keyboard_Class::KeysState &state) {
  if (c == '`') {
    return false;
  }
  return true;
}
