/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <M5Cardputer.h>
#include <M5Unified.h>

#include <random>

constexpr int swidth = 240;
constexpr int sheight = 135;

constexpr int sq_size = 10;

constexpr int sq_wide = swidth / sq_size - 4;
constexpr int sq_high = sheight / sq_size - 1;

std::mt19937 rng((std::random_device())());

// returns a random int in the range [0,i)
int rand_int(int i) {
  std::uniform_int_distribution<std::mt19937::result_type> dist(0, i - 1);
  return dist(rng);
}

struct State {
  State() {
    for (int y = 0; y < sq_high; ++y) {
      for (int x = 0; x < sq_wide; ++x) {
        grid[y][x] = ' ';
      }
    }
  }
  char grid[sq_high][sq_wide];
  std::vector<int32_t> stack;
  bool stringmode = false;
  int x = 0;
  int y = 0;
  int dx = 1;
  int dy = 0;

  int32_t pop() {
    if (stack.size()) {
      int32_t ret = stack.back();
      stack.pop_back();
      return ret;
    }
    return 0;
  }

  void push(int32_t i) { stack.push_back(i); }

  void step() {
    int32_t a, b;
    if (stringmode) {
      if (grid[y][x] == '"') {
        stringmode = false;
      } else {
        push(grid[y][x]);
      }
    } else {
      switch (grid[y][x]) {
        case '+':
          push(pop() + pop());
          break;
        case '-':
          b = pop();
          a = pop();
          push(a - b);
          break;
        case '*':
          push(pop() * pop());
          break;
        case '/':
          b = pop();
          a = pop();
          push(a / b);
          break;
        case '%':
          b = pop();
          a = pop();
          push(a %
               b);  // would prefer positive result if a is negative, but ehh
          break;
        case '!':
          push(!pop());
          break;
        case '`':
          b = pop();
          a = pop();
          push(a > b);
          break;
        case '>':
          dx = 1;
          dy = 0;
          break;
        case '<':
          dx = -1;
          dy = 0;
          break;
        case '^':
          dx = 0;
          dy = -1;
          break;
        case 'v':
          dx = 0;
          dy = 1;
          break;
        case '?':
          switch (rand_int(4)) {
            case 0:
              dx = -1;
              dy = 0;
            case 1:
              dx = 1;
              dy = 0;
            case 2:
              dx = 0;
              dy = -1;
            case 3:
              dx = 0;
              dy = 1;
          }
          break;
        case '_':
          dy = 0;
          if (pop()) {
            dx = -1;
          } else {
            dx = 1;
          }
          break;
        case '|':
          dx = 0;
          if (pop()) {
            dy = -1;
          } else {
            dy = 1;
          }
          break;
        case '"':
          stringmode = true;
          break;
        case ':':
          a = pop();
          push(a);
          push(a);
          break;
        case '\\':
          b = pop();
          a = pop();
          push(b);
          push(a);
          break;
        case '$':
          pop();
          break;
        case '.':
          // TODO
          break;
        case ',':
          // TODO
          break;
        case '#':
          x += dx;
          y += dy;
          break;
        case 'g':
          b = pop();
          a = pop();
          push(grid[b][a]);
          break;
        case 'p':
          b = pop();
          a = pop();
          grid[b][a] = pop();
          break;
        case '&':
          // TODO
          break;
        case '~':
          // TODO
          break;
        case '@':
          exit(0);
          break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          push(grid[y][x] - '0');
          break;
      }
    }
    x += dx;
    y += dy;
  }

  void draw() {
    m5gfx::M5Canvas disp(&M5Cardputer.Display);
    disp.createSprite(swidth, sheight);
    // disp.setTextSize(sq_size);
    disp.setTextColor(BLACK, WHITE);
    disp.clear(WHITE);
    for (int x = 0; x < sq_wide; ++x) {
      for (int y = 0; y < sq_high; ++y) {
        char tile = grid[y][x];
        if ((tile >= 32) && (tile <= 126)) {
          disp.drawChar(tile, sq_size * x + 3, sq_size * y + 2);
        }
      }
    }
    for (int x = 0; x <= sq_wide; ++x) {
      // vertical lines
      disp.drawFastVLine(x * sq_size, 0, sq_high * sq_size + 1, BLACK);
    }
    for (int y = 0; y <= sq_high; ++y) {
      // horizontal lines
      disp.drawFastHLine(0, y * sq_size, sq_wide * sq_size + 1, BLACK);
    }
    disp.drawRect(sq_size * x + 1, sq_size * y + 1, sq_size - 1, sq_size - 1,
                  BLUE);
    int offset = (sheight - sq_size * sq_high - 10) / 2;
    disp.drawString(("dx: " + std::to_string(dx)).c_str(), offset,
                    sq_size * sq_high + offset);
    disp.drawString(("dy: " + std::to_string(dy)).c_str(), offset + 50,
                    sq_size * sq_high + offset);

    disp.drawString("stack", sq_size * sq_wide + 3, 0);
    disp.drawFastHLine(sq_size * sq_wide + 2, 10,
                       swidth - (sq_size * sq_wide) - 4, BLACK);
    for (int i = 0; i < stack.size(); ++i) {
      disp.drawString(std::to_string(stack[i]).c_str(),
                      sq_size * sq_wide + offset, 10 * i + 12);
    }
    disp.pushSprite(0, 0);
  }
};

extern "C" void app_main() {
  M5Cardputer.begin(true);

  std::vector<char> last;
  std::string word_chars;

  State st;
  st.draw();
  bool running = false;
  while (true) {
    if (M5Cardputer.Keyboard.isChange()) {
      const auto &keys = M5Cardputer.Keyboard.keysState();
      bool last_tab = false;
      bool last_enter = false;
      if (keys.tab && !last_tab) {
        running = false;
        st.step();
        st.draw();
      }
      if (keys.enter && !last_enter) {
        running = true;
      }
      // don't reprocess keys on shift-up or shift-down
      for (char c : keys.word) {
        // TODO releasing shift while still holding a character
        // gives an extra keypress
        // it also stops it from running too
        // example: shift+a+enter -> let go of shift
        if (std::find(last.begin(), last.end(), c) == last.end()) {
          running = false;
          if (c == '/' && !keys.fn) {
            // right
            st.dx = 1;
            st.dy = 0;
            st.x += 1;
          } else if (c == ',' && !keys.fn) {
            // left
            st.dx = -1;
            st.dy = 0;
            st.x -= 1;
          } else if (c == ';' && !keys.fn) {
            // up
            st.dx = 0;
            st.dy = -1;
            st.y -= 1;
          } else if (c == '.' && !keys.fn) {
            // down
            st.dx = 0;
            st.dy = 1;
            st.y += 1;
          } else if ((c >= 32) && (c <= 126)) {
            st.grid[st.y][st.x] = c;
          }
        }
        st.draw();
      }
      last = keys.word;
      last_tab = keys.tab;
      last_enter = keys.enter;
    }
    if (running) {
      st.step();
      st.draw();
      M5.delay(50);
    } else {
      M5.delay(1);
    }
    M5Cardputer.update();
  }
}
