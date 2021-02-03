#pragma once

#include <chrono>

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdint>
#include <climits>

#include <assert.h>

class NullBuffer : public std::streambuf {
  public:
    int overflow(int c) { return c; }
};

class Board {
public:
  constexpr static int kMax = 4;
  constexpr static int kRows = kMax;
  constexpr static int kCols = kMax;

  static_assert(kRows == kCols, "Grid must be a square");

  enum class Direction : int {
    kUnknown,
    kUp,
    kDown,
    kLeft,
    kRight,
  };

  enum class Operation : int {
    kUnknown,
    kSlide,
    kCombine,
    kCompress,
  };

  Board(unsigned int seed = 0) {
    if (seed == 0) {
      using hrc = std::chrono::high_resolution_clock;
      using nano = std::chrono::nanoseconds;

      std::chrono::time_point<hrc> now_tp = hrc::now();
      auto now_dur = now_tp.time_since_epoch();

      auto now_ns = std::chrono::duration_cast<nano>(now_dur).count();
      seed = now_ns % UINT_MAX;
    }

    std::srand(seed);

    for (int r = 0; r < kRows; r++) {
      for (int c = 0; c < kCols; c++) {
        board[r][c] = 0;
      }
    }

    // game starts with 2 random values
    insertRandomValue();
    insertRandomValue();
  }

  ~Board() {}

  int val(int r, int c) {
    if (! ((0 <= r && r < kRows) && (0 <= c && c < kCols))) {
      return -1;
    }

    return board[r][c];
  }

  bool insertRandomValue();

  static void debug(const char *s, int arr[], int i, int j) {
    NullBuffer null_buffer;
    std::ostream null(&null_buffer);

    std::ostream& stream = null;
    stream << "[" << s << "] ";

    if (i < kMax) {
      stream << "a[" << i << "] = " << arr[i];
    } else {
      stream << "---";
    }

    stream << ", ";

    if (j < kMax) {
      stream << "a[" << j << "] = " << arr[j];
    } else {
      stream << "---";
    }

    stream << std::endl;
  }

  static bool compress(int arr[]) {
    bool did_operation = false;

    int i = 0;
    int j = 1;

    while (j < kMax) {
      debug("top", arr, i, j);
      assert(i != j);

      // ensure that the next non-zero value is at [i]
      while (arr[i] == 0 && j < kMax) {
        debug("finding nonzero", arr, i, j);
        if (arr[j] != 0) {
          // swap [i] and [j]; effectively performs "sliding"
          int temp = arr[i];
          arr[i] = arr[j];
          arr[j] = temp;

          debug("swap", arr, i, j);
          did_operation = true;
        }

        j++;
      }

      debug("found nonzero", arr, i, j);

      for (int check = i+1; check < j; check++) {
        assert(arr[check] == 0);
      }

      if (j == kMax) {
        // [i] may or may not be 0
        break;
      }
      assert(arr[i] != 0);

      // look for next nonzero to see if combining is possible
      while (arr[j] == 0 && j < kMax) {
        j++;
      }
      debug("found next nonzero", arr, i, j);
      if (j == kMax) {
        // no other number found
        break;
      }
      assert(arr[j] != 0);

      if (arr[i] == arr[j]) {
        debug("combining", arr, i, j);

        arr[i] *= 2;
        arr[j] = 0;

        // a combination has occurred; done with finalizing index i
        did_operation = true;
      }

      i++;
      j = i + 1;

      debug("next", arr, i, j);
    }

    debug("done", arr, i, j);

    bool nonzero = true;
    for (int idx = 0; idx < kMax; idx++) {
      if (!nonzero)
        assert(arr[idx] == 0);

      if (nonzero && arr[idx] == 0) {
        nonzero = false;
      }
    }

    return did_operation;
  }

  static bool simple_combine(int arr[]) {
    return false;
  }

  static bool simple_slide(int arr[]) {
    return false;
  }

  static void convert(int start, int stride, int i, int& r, int &c) {
    int idx = start + i*stride;
    r = idx / kRows;
    c = idx % kCols;
  }

  bool combine(Direction d);
  bool slide(Direction d);
  bool compress(Direction d);
  bool operation(Direction d, Operation o);

  void pack(int start, int stride, int arr[]);
  void unpack(int start, int stride, int arr[]);

  uint16_t emptyCells(void);

  bool checkGameOver() {
    if (emptyCells() > 0)
      return false;

    for (int r = 0; r < kRows; r++) {
      for (int c = 0; c < kCols-1; c++) {
        if (board[r][c] == board[r][c+1]) {
          return false;
        }
      }
    }

    for (int c = 0; c < kCols; c++) {
      for (int r = 0; r < kRows-1; r++) {
        if (board[r][c] == board[r+1][c]) {
          return false;
        }
      }
    }

    return true;
  }

  bool applyMove(Direction d) {
    bool ret = compress(d);

    insertRandomValue();

    return ret;
  }

  static int countBits(uint16_t bits) {
    bits = bits - ((bits >> 1) & 0x55555555);
    bits = (bits & 0x33333333) + ((bits >> 2) & 0x33333333);
    return ((bits + (bits >> 4) & 0x0F0F0F0F) * 0x01010101) >> 24;
  }

  friend std::ostream& operator<<(std::ostream& stream, Board& b);

private:
  int board[kRows][kCols];
};
