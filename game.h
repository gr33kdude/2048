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

  // in any 4-cell row or column, let the values be:
  // [ a, b, c, d ]
  static void hw_combine(int arr[]) {
    int a = arr[0];
    int b = arr[1];
    int c = arr[2];
    int d = arr[3];

    bool AB = a == b;
    bool BC = b == c;
    bool CD = c == d;

    int A = AB ? 2*a : a;
    int B = AB ? (CD ? 2*c : c) \
           : (BC ? 2*b : b);
    int C = AB ? (CD ? 0 : d) \
               : (BC ? d : (CD ? 2*c : d));
    int D = (AB && BC && CD) ? 0 : d;

    arr[0] = A;
    arr[1] = B;
    arr[2] = C;
    arr[3] = D;
  }
  
  static void hw_slide(int arr[]) {
    int a = arr[0];
    int b = arr[1];
    int c = arr[2];
    int d = arr[3];

    bool AA = a > 0;
    bool BB = b > 0;
    bool CC = c > 0;
    bool DD = d > 0;

    bool AB = AA && BB;
    bool BC = BB && CC;
    bool CD = CC && DD;
    bool AC = AA && CC;
    bool ABC = AB && CC;
    bool ABCD = AB && CD;

    int A = AA ? a \
         : (BB ? b \
         : (CC ? c \
         : (DD ? d : 0)));
    int B = AB ? b \
         : (CC && (AA != BB)) ? c \
         : (DD && (AA != BB != CC)) ? d \
         : 0;
    int C = ABC ? c \
         : (DD && ((!AA && BC) || (!BB && AC) || (!CC && AB))) ? d \
         : 0;
    int D = ABCD ? d : 0;

    arr[0] = A;
    arr[1] = B;
    arr[2] = C;
    arr[3] = D;
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

#if 0
  void combine(Direction d) {
    for (int i = 0; i < 4; i++) {

      typedef struct pos {
        int r;
        int c;
      } pos;
      pos first  { .r = 0, .c = 0 };
      pos second { .r = 0, .c = 0 };

      switch (d) {
        case Direction::kUp:
          first  = { .r = 0, .c = i };
          second = { .r = 2, .c = i };
          
          first_c = i;

          if (board[0][i] == board[1][i]) {
            board[0][i] *= 2;
            board[1][i] = 0;
          }
          if (board[2][i] == board[3][i]) {
            board[2][i] *= 2;
            board[3][i] = 0;
          }
          break;
        case Direction::kDown:
          if (board[3][i] == board[2][i]) {
            board[3][i] *= 2;
            board[2][i] = 0;
          }
          if (board[1][i] == board[0][i]) {
            board[1][i] *= 2;
            board[0][i] = 0;
          }
          break;
        case Direction::kLeft:
          if (board[i][0] == board[i][1]) {
            board[i][0] *= 2;
            board[i][1] = 0;
          }

          if (board[i][2] == board[i][3]) {
            board[i][2] *= 2;
            board[i][3] = 0;
          }
          break;
        case Direction::kDown:
          if (board[r][i] == board[r+1][i]) {
            board[r][i] *= 2;
            board[r+1][i] = 0;
          }

          if (board[r+2][i] == board[r+3][i]) {
            board[r+2][i] *= 2;
            board[r+3][i] = 0;
          }
          break;
      }
    }
  }
#endif

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
