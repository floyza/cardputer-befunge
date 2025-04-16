/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <M5Cardputer.h>
#include <M5Unified.h>

#include <random>

#include "esp_log.h"
#include "esp_spiffs.h"

static const char* TAG = "befunge";

constexpr int swidth = 240;
constexpr int sheight = 135;

constexpr int sq_size = 10;

// visible width and height
constexpr int sq_wide = swidth / sq_size - 4;
constexpr int sq_high = sheight / sq_size - 1;

constexpr int grid_wide = 256;
constexpr int grid_high = 256;

std::mt19937 rng((std::random_device())());

// returns a random int in the range [0,i)
int rand_int(int i) {
  std::uniform_int_distribution<std::mt19937::result_type> dist(0, i - 1);
  return dist(rng);
}

int mod(int i, int m) {
  int s = i % m;
  return (s >= 0) ? s : (s + m);
}

struct State {
 private:
  int16_t grid[grid_high][grid_wide];  // access with idx()
 public:
  State() {
    for (int y = 0; y < grid_high; ++y) {
      for (int x = 0; x < grid_wide; ++x) {
        grid[y][x] = ' ';
      }
    }
  }
  std::vector<int16_t> stack;
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
      if (idx(x, y) == '"') {
        stringmode = false;
      } else {
        push(idx(x, y));
      }
    } else {
      switch (idx(x, y)) {
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
              break;
            case 1:
              dx = 1;
              dy = 0;
              break;
            case 2:
              dx = 0;
              dy = -1;
              break;
            case 3:
              dx = 0;
              dy = 1;
              break;
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
          push(idx(a, b));
          break;
        case 'p':
          b = pop();
          a = pop();
          idx(a, b) = pop();
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
    advance_pointer();
  }

  void advance_pointer() {
    x = mod(x + dx, grid_wide);
    y = mod(y + dy, grid_high);
  }

  int16_t& idx(int x, int y) {
    return grid[mod(y, grid_high)][mod(x, grid_wide)];
  }

  const int16_t& idx(int x, int y) const {
    return grid[mod(y, grid_high)][mod(x, grid_wide)];
  }

  // draw grid focused on x,y
  void draw() const {
    m5gfx::M5Canvas disp(&M5Cardputer.Display);
    disp.createSprite(swidth, sheight);
    // disp.setTextSize(sq_size);
    disp.setTextColor(BLACK, WHITE);
    disp.clear(WHITE);
    int bx = x - sq_wide / 2;
    int by = y - sq_high / 2;
    for (int tx = 0; tx < sq_wide; ++tx) {
      for (int ty = 0; ty < sq_high; ++ty) {
        char tile = idx(tx + bx, ty + by);
        // disp.fillRect(sq_size * tx + 1, sq_size * ty + 1, sq_size - 1,
        //               sq_size - 1, LIGHTGREY);
        if ((tile >= 33) && (tile <= 126)) {
          disp.drawChar(tile, sq_size * tx + 3, sq_size * ty + 2);
        } else if (tile == 32) {
          // space
        } else {
          disp.drawChar(sq_size * tx + 3, sq_size * ty + 2, '?', WHITE, RED, 1);
        }
      }
    }
    for (int tx = 0; tx <= sq_wide; ++tx) {
      // vertical lines
      if (mod(tx + bx, grid_wide) == 0) {
        disp.drawFastVLine(tx * sq_size, 0, sq_high * sq_size + 1, BLACK);
        disp.drawFastVLine(tx * sq_size - 1, 0, sq_high * sq_size + 1, BLACK);
        disp.drawFastVLine(tx * sq_size + 1, 0, sq_high * sq_size + 1, BLACK);
      } else {
        disp.drawFastVLine(tx * sq_size, 0, sq_high * sq_size + 1, BLACK);
      }
    }
    for (int ty = 0; ty <= sq_high; ++ty) {
      // horizontal lines
      if (mod(ty + by, grid_high) == 0) {
        disp.drawFastHLine(0, ty * sq_size, sq_wide * sq_size + 1, BLACK);
        disp.drawFastHLine(0, ty * sq_size - 1, sq_wide * sq_size + 1, BLACK);
        disp.drawFastHLine(0, ty * sq_size + 1, sq_wide * sq_size + 1, BLACK);
      } else {
        disp.drawFastHLine(0, ty * sq_size, sq_wide * sq_size + 1, BLACK);
      }
    }
    disp.drawRect(sq_size * (sq_wide / 2) + 1, sq_size * (sq_high / 2) + 1,
                  sq_size - 1, sq_size - 1, BLUE);
    int offset = (sheight - sq_size * sq_high - 10) / 2;
    disp.drawString(("value: " + std::to_string(idx(x, y))).c_str(), offset,
                    sq_size * sq_high + offset);
    disp.drawString(("dx: " + std::to_string(dx)).c_str(), offset + 80,
                    sq_size * sq_high + offset);
    disp.drawString(("dy: " + std::to_string(dy)).c_str(), offset + 130,
                    sq_size * sq_high + offset);

    disp.drawString(
        ("%: " + std::to_string(M5Cardputer.Power.getBatteryLevel())).c_str(),
        sq_size * sq_wide + offset, sq_size * sq_high + offset - sq_size * 1);
    disp.drawString(
        ("mV" + std::to_string(M5Cardputer.Power.getBatteryVoltage())).c_str(),
        sq_size * sq_wide + offset, sq_size * sq_high + offset - sq_size * 0);

    disp.drawString("stack", sq_size * sq_wide + 3, 0);
    disp.drawFastHLine(sq_size * sq_wide + 2, 10,
                       swidth - (sq_size * sq_wide) - 4, BLACK);
    for (int i = 0; i < stack.size(); ++i) {
      disp.drawString(std::to_string(stack[i]).c_str(),
                      sq_size * sq_wide + offset, 10 * i + 12);
    }
    disp.pushSprite(0, 0);
  }

  void save() const {
    ESP_LOGI(TAG, "Saving program");
    {
      m5gfx::M5Canvas disp(&M5Cardputer.Display);
      disp.createSprite(swidth, sheight);
      disp.setTextColor(BLACK, WHITE);
      disp.clear(WHITE);
      disp.drawString("SAVING", swidth / 4, sheight / 4);
      disp.pushSprite(0, 0);
    }
    FILE* f = fopen("/spiffs/prog", "w");
    if (f == nullptr) {
      ESP_LOGE(TAG, "Failed to open file for writing");
      return;
    }
    for (int y = 0; y < grid_high; ++y) {
      for (int x = 0; x < grid_wide; ++x) {
        fprintf(f, "%c%c", grid[y][x] & 0xff, grid[y][x] >> 8);
      }
    }
    fclose(f);
    draw();
  }

  void load() {
    ESP_LOGI(TAG, "Loading program");
    {
      m5gfx::M5Canvas disp(&M5Cardputer.Display);
      disp.createSprite(swidth, sheight);
      disp.setTextColor(BLACK, WHITE);
      disp.clear(WHITE);
      disp.drawString("LOADING", swidth / 4, sheight / 4);
      disp.pushSprite(0, 0);
    }
    FILE* f = fopen("/spiffs/prog", "r");
    if (f == nullptr) {
      ESP_LOGE(TAG, "Failed to open file for reading");
      return;
    }
    for (int y = 0; y < grid_high; ++y) {
      for (int x = 0; x < grid_wide; ++x) {
        int lsb = fgetc(f);
        int msb = fgetc(f);
        if (lsb == EOF || msb == EOF) {
          ESP_LOGE(TAG, "Hit EOF while loading program");
          return;
        }
        grid[y][x] = lsb | (msb << 8);
      }
    }
    fclose(f);
    draw();
  }
};

void init_spiffs() {
  ESP_LOGI(TAG, "Initializing SPIFFS");
  esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = nullptr,
      .max_files = 5,
      .format_if_mount_failed = true,
  };

  esp_err_t ret = esp_vfs_spiffs_register(&conf);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount or format filesystem");
    } else if (ret == ESP_ERR_NOT_FOUND) {
      ESP_LOGE(TAG, "Failed to find SPIFFS partition");
    } else {
      ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
    }
    return;
  }

  ret = esp_spiffs_check(conf.partition_label);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "spiffs_check() failed (%s)", esp_err_to_name(ret));
    return;
  } else {
    ESP_LOGI(TAG, "spiffs_check() successful");
  }

  size_t total = 0, used = 0;
  ret = esp_spiffs_info(conf.partition_label, &total, &used);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)",
             esp_err_to_name(ret));
    return;
  } else {
    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
  }
}

extern "C" void app_main() {
  ESP_LOGI(TAG, "Starting");

  M5Cardputer.begin(true);
  if (!M5Cardputer.Power.begin()) {
    ESP_LOGE(TAG, "Failed to init power.");
  }
  init_spiffs();

  ESP_LOGI(TAG, "Init done: starting");

  std::vector<char> last;
  std::string word_chars;

  M5Cardputer.Power.getBatteryLevel();

  State* st = new State();
  st->draw();
  bool running = false;
  int ticks_since_last_draw = 0;
  while (true) {
    if (M5Cardputer.Keyboard.isChange()) {
      const auto& keys = M5Cardputer.Keyboard.keysState();
      bool last_tab = false;
      bool last_enter = false;
      bool redraw = false;
      if (keys.tab && !last_tab) {
        running = false;
        st->step();
        redraw = true;
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
          bool changed = true;
          if (c == '/' && !keys.fn) {
            // right
            st->dx = 1;
            st->dy = 0;
            st->advance_pointer();
          } else if (c == ',' && !keys.fn) {
            // left
            st->dx = -1;
            st->dy = 0;
            st->advance_pointer();
          } else if (c == ';' && !keys.fn) {
            // up
            st->dx = 0;
            st->dy = -1;
            st->advance_pointer();
          } else if (c == '.' && !keys.fn) {
            // down
            st->dx = 0;
            st->dy = 1;
            st->advance_pointer();
          } else if (c == 's' && keys.fn) {
            st->save();
          } else if (c == 'l' && keys.fn) {
            st->load();
          } else if ((c >= 32) && (c <= 126)) {
            st->idx(st->x, st->y) = c;
          } else {
            changed = false;
          }
          if (changed) {
            redraw = true;
          }
        }
      }
      if (redraw) {
        st->draw();
        ticks_since_last_draw = 0;
      }
      last = keys.word;
      last_tab = keys.tab;
      last_enter = keys.enter;
    }
    if (running) {
      st->step();
      st->draw();
      ticks_since_last_draw = 0;
      M5.delay(50);
    } else {
      M5.delay(10);
      ticks_since_last_draw++;
    }
    if (ticks_since_last_draw >= 200) {
      // every 5 seconds
      st->draw();
      ticks_since_last_draw = 0;
    }
    M5Cardputer.update();
  }
}
