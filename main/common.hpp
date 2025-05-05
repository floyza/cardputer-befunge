#ifndef COMMON_H_
#define COMMON_H_

#include <M5Cardputer.h>
#include <M5Unified.h>

#include <random>

constexpr int swidth = 240;
constexpr int sheight = 135;

constexpr int sq_size = 10;

// visible width and height
constexpr int sq_wide = swidth / sq_size - 4;
constexpr int sq_high = sheight / sq_size - 1;

constexpr int grid_wide = 128;
constexpr int grid_high = 128;

static std::mt19937 rng;

// returns a random int in the range [0,i)
inline int rand_int(int i) {
  std::uniform_int_distribution<std::mt19937::result_type> dist(0, i - 1);
  return dist(rng);
}

inline int mod(int i, int m) {
  int s = i % m;
  return (s >= 0) ? s : (s + m);
}

#endif  // COMMON_H_
