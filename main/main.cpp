/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <M5Cardputer.h>
#include <M5Unified.h>

#include <random>

#include "driver/sdmmc_host.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

static const char* TAG = "befunge";

constexpr int SD_SPI_SCK_PIN = 40;
constexpr int SD_SPI_MISO_PIN = 39;
constexpr int SD_SPI_MOSI_PIN = 14;
constexpr int SD_SPI_CS_PIN = 12;

constexpr int swidth = 240;
constexpr int sheight = 135;

constexpr int sq_size = 10;

// visible width and height
constexpr int sq_wide = swidth / sq_size - 4;
constexpr int sq_high = sheight / sq_size - 1;

constexpr int grid_wide = 128;
constexpr int grid_high = 128;

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
  int64_t grid[grid_high][grid_wide];  // access with idx()
 public:
  State() { clear(); }
  std::vector<int64_t> stack;
  bool stringmode = false;
  bool helpmode = false;
  int x = 0;
  int y = 0;
  int dx = 1;
  int dy = 0;

  void clear() {
    for (int y = 0; y < grid_high; ++y) {
      for (int x = 0; x < grid_wide; ++x) {
        grid[y][x] = ' ';
      }
    }
  }

  int64_t pop() {
    if (stack.size()) {
      int64_t ret = stack.back();
      stack.pop_back();
      return ret;
    }
    return 0;
  }

  void push(int64_t i) { stack.push_back(i); }

  void step() {
    int64_t a, b;
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

  int64_t& idx(int x, int y) {
    return grid[mod(y, grid_high)][mod(x, grid_wide)];
  }

  const int64_t& idx(int x, int y) const {
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
    int offset = 2;
    disp.drawString(("value: " + std::to_string(idx(x, y))).c_str(), offset,
                    sq_size * sq_high + offset);

    disp.drawString("memf", sq_size * sq_wide + offset,
                    sq_size * sq_high + offset - sq_size * 2);
    disp.drawString(std::to_string(esp_get_free_heap_size()).c_str(),
                    sq_size * sq_wide + offset,
                    sq_size * sq_high + offset - sq_size * 1);
    disp.drawString(
        ("b%:" + std::to_string(M5Cardputer.Power.getBatteryLevel())).c_str(),
        sq_size * sq_wide + offset, sq_size * sq_high + offset - sq_size * 0);

    disp.drawString("stack", sq_size * sq_wide + 3, 0);
    disp.drawFastHLine(sq_size * sq_wide + 2, 10,
                       swidth - (sq_size * sq_wide) - 4, BLACK);
    constexpr int max_stack_shown = 9;  // determined experimentally
    int first_stack_shown = 0;
    if (stack.size() > max_stack_shown) {
      first_stack_shown = stack.size() - max_stack_shown;
    }
    int i = 0;
    for (int si = first_stack_shown; si < stack.size(); ++si) {
      if (first_stack_shown != 0 && i == 0) {
        disp.drawString("^^^^^^", sq_size * sq_wide + offset, 10 * i + 12);
        i++;
        continue;
      }
      std::string val = std::to_string(stack[si]);
      if (val.size() > 6) {
        val = val.substr(0, 5) + ">";
      }
      disp.drawString(val.c_str(), sq_size * sq_wide + offset, 10 * i + 12);
      i++;
    }
    disp.pushSprite(0, 0);

    if (helpmode) {
      disp.createSprite(swidth - 20, sheight - 20);
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
      disp.pushSprite(10, 10);
    }
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
    FILE* f = fopen("/sdcard/prog", "w");
    if (f == nullptr) {
      ESP_LOGE(TAG, "Failed to open file for writing");
      return;
    }
    for (int y = 0; y < grid_high; ++y) {
      for (int x = 0; x < grid_wide; ++x) {
        for (int i = 0; i < sizeof(**grid); ++i) {
          fprintf(f, "%c", static_cast<char>((grid[y][x] >> (i * 8)) & 0xff));
          if (y == 0 && x == 0) {
            ESP_LOGI(TAG, "%s",
                     (std::string("save ") + std::to_string(grid[y][x]) +
                      std::string(" as ") +
                      std::to_string((grid[y][x] >> (i * 8)) & 0xff))
                         .c_str());
          }
        }
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
    FILE* f = fopen("/sdcard/prog", "r");
    if (f == nullptr) {
      ESP_LOGE(TAG, "Failed to open file for reading");
      return;
    }
    for (int y = 0; y < grid_high; ++y) {
      for (int x = 0; x < grid_wide; ++x) {
        int64_t pt = 0;
        for (int i = 0; i < sizeof(**grid); ++i) {
          int64_t val = fgetc(f);
          if (val == EOF) {
            ESP_LOGE(TAG, "Hit EOF while loading program");
            return;
          }
          pt |= val << (8 * i);
          if (y == 0 && x == 0) {
            ESP_LOGI(TAG, "%i", i);
            ESP_LOGI(TAG, "%s",
                     (std::string("load ") + std::to_string(val) +
                      std::string(" as ") + std::to_string(val << (8 * i)))
                         .c_str());
          }
        }
        grid[y][x] = pt;
      }
    }
    fclose(f);
    draw();
  }
};

void init_sd() {
  esp_err_t ret;

  esp_vfs_fat_sdmmc_mount_config_t mount_cfg{
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024,
  };

  sdmmc_card_t* card;
  const char* mount_point = "/sdcard";
  ESP_LOGI(TAG, "Initializing SD card");

  ESP_LOGI(TAG, "Using SPI peripheral");

  sdmmc_host_t host = SDSPI_HOST_DEFAULT();

  spi_bus_config_t buf_cfg{
      .mosi_io_num = SD_SPI_MOSI_PIN,
      .miso_io_num = SD_SPI_MISO_PIN,
      .sclk_io_num = SD_SPI_SCK_PIN,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 4000,
  };

  ret = spi_bus_initialize(static_cast<spi_host_device_t>(host.slot), &buf_cfg,
                           SDSPI_DEFAULT_DMA);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init bus");
    return;
  }

  sdspi_device_config_t slot_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_cfg.gpio_cs = static_cast<gpio_num_t>(SD_SPI_CS_PIN);
  slot_cfg.host_id = static_cast<spi_host_device_t>(host.slot);

  ESP_LOGI(TAG, "Mounting filesystem");
  ret =
      esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_cfg, &mount_cfg, &card);
  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount filesystem (auto-format is disabled)");
    } else {
      ESP_LOGE(TAG, "Failed to initialize the card (%s)", esp_err_to_name(ret));
    }
  }
  ESP_LOGI(TAG, "Filesystem mounted");

  sdmmc_card_print_info(stdout, card);
}

extern "C" void app_main() {
  ESP_LOGI(TAG, "Starting");

  M5Cardputer.begin(true);
  if (!M5Cardputer.Power.begin()) {
    ESP_LOGE(TAG, "Failed to init power.");
  }
  init_sd();

  ESP_LOGI(TAG, "Init done: starting");

  std::vector<char> last;
  std::string word_chars;

  State* st = new State();
  // for some reason the display locks up if we don't draw before we load
  st->draw();
  st->load();
  bool running = false;
  int ticks_since_last_draw = 0;
  while (true) {
    if (M5Cardputer.Keyboard.isChange()) {
      const auto& keys = M5Cardputer.Keyboard.keysState();
      bool redraw = false;
      if (keys.tab) {
        running = false;
        st->step();
        redraw = true;
      }
      if (keys.enter) {
        running = true;
      }
      // don't reprocess keys on shift-up or shift-down
      for (char c : keys.word) {
        // TODO releasing shift while still holding a character
        // gives an extra keypress
        // it also stops it from running too
        // example: shift+a+enter -> let go of shift
        if (std::find(last.begin(), last.end(), c) == last.end()) {
          if (st->helpmode) {
            if (c == '`') {
              st->helpmode = false;
              redraw = true;
            }
            continue;
          }
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
          } else if (c == ' ' && keys.fn) {
            st->clear();
          } else if (c == 'c' && keys.fn) {
            st->stack.clear();
          } else if (c == 'h' && keys.fn) {
            st->helpmode = true;
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
