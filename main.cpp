#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <climits>

#define NCURSES 0

#if NCURSES
# include <curses.h>
#else
# include <termios.h>
#endif

class Board {
public:
  constexpr static int kRows = 4;
  constexpr static int kCols = 4;

  enum class Direction : int {
    kUnknown,
    kUp,
    kDown,
    kLeft,
    kRight,
  };

  Board() {
    using hrc = std::chrono::high_resolution_clock;
    using nano = std::chrono::nanoseconds;

    std::chrono::time_point<hrc> now_tp = hrc::now();
    auto now_dur = now_tp.time_since_epoch();

    auto now_ns = std::chrono::duration_cast<nano>(now_dur).count();
    unsigned int seed = now_ns % UINT_MAX;

    std::srand(seed);

    for (int r = 0; r < kRows; r++) {
      for (int c = 0; c < kCols; c++) {
        board[r][c] = 0;
      }
    }

    for (int i = 0; i < 2; i++) {
      insertRandomValue();
    }
  }

  ~Board() {}

  int val(int r, int c) {
    if (! ((0 <= r && r < kRows) && (0 <= c && c < kCols))) {
      return -1;
    }

    return board[r][c];
  }

  void insertRandomValue() {
    uint16_t empty = emptyCells();
    int num_empty = countBits(empty);
    if (num_empty == 0) 
      return;

    int insert_spot = std::rand() % num_empty;
    // 10% chance of 4, otherwise 2
    int insert_value = ((std::rand() % 10) == 0) ? 4 : 2;

    int empty_passed = 0;
    bool done = false;
    for (int r = 0; !done && r < kRows; r++) {
      for (int c = 0; !done && c < kCols; c++) {
        if (board[r][c] != 0)
          continue;

        if (empty_passed == insert_spot) {
          done = true;
          board[r][c] = insert_value; 
          break;
        }

        empty_passed++;
      }
    }

    if (empty_passed != insert_spot)
      std::cout << "WAT" << std::endl;
  }

  // in any 4-cell row or column, let the values be:
  // [ a, b, c, d ]
  static void simple_combine(int arr[]) {
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
  
  static void simple_slide(int arr[]) {
    int a = arr[0];
    int b = arr[1];
    int c = arr[2];
    int d = arr[3];

    bool AA = a > 0;
    bool BB = b > 0;
    bool CC = c > 0;
    bool DD = d > 0;

    bool AB = AA && BB;
    bool CD = CC && DD;
    bool ABCD = AB && CD;

    int A = AA ? a \
         : (BB ? b \
         : (CC ? c \
         : (DD ? d : 0)));
    int B = AB ? b \
         : (CC && (AA != BB)) ? c \
         : (DD && !AB) ? d \
         : 0;
    int C = (AB && CC) ? c \
         : ((CD && !AB) || ABCD) ? d \
         : 0;
    int D = ABCD;

    arr[0] = A;
    arr[1] = B;
    arr[2] = C;
    arr[3] = D;
  }

  void convert(int start, int stride, int i, int& r, int &c) {
    int idx = start + i*stride;
    r = idx / kRows;
    c = idx % kCols;
  }

  void pack(int start, int stride, int arr[]) {
    for (int i = 0; i < 4; i++) {
      int r, c;
      convert(start, stride, i, r, c);
      arr[i] = board[r][c];
    }
  }

  void unpack(int start, int stride, int arr[]) {
    for (int i = 0; i < 4; i++) {
      int r, c;
      convert(start, stride, i, r, c);
      board[r][c] = arr[i];
    }
  }

#if 1
  void combine(Direction d) {
    for (int i = 0; i < 4; i++) {
      int arr[4] = {};
      int start = 0;
      int stride = 0;

      switch (d) {
        case Direction::kUp:
          start  = 12 + i;
          stride = -4;
          break;
        case Direction::kDown:
          start  = 0 + i;
          stride = 4;
          break;
        case Direction::kLeft:
          start  = 4 * i;
          stride = 1;
          break;
        case Direction::kRight:
          start  = 4 * i - 1;
          stride = -1;
          break;
      }

      pack(start, stride, arr);
      simple_combine(arr);
      unpack(start, stride, arr);
    }
  }
#else
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
          first_r = 0;
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

  // for each of the nonzero final tiles, scan for the next nonzero
  // if equal, combine, otherwise 
  void slide(Direction d) {
    for (int i = 0; i < 4; i++) {
      switch (d) {
        case Direction::kUp:
          for (int r = 0; r < kRows-1; r++) {
            if (board[r][i] == 0) {
              board[r][i] = board[r+1][i];
              board[r+1][i] = 0;
            }
          }
          break;
        case Direction::kDown:
          for (int r = kRows-1; r > 0; r--) {
            if (board[r][i] == 0) {
              board[r][i] = board[r-1][i];
              board[r-1][i] = 0;
            }
          }
          break;
        case Direction::kLeft:
          for (int c = 0; c < kCols-1; c++) {
            if (board[i][c] == 0) {
              board[i][c] = board[i][c+1];
              board[i][c+1] = 0;
            }
          }
          break;
        case Direction::kRight:
          for (int c = kCols-1; c > 0; c--) {
            if (board[i][c] == 0) {
              board[i][c] = board[i][c-1];
              board[i][c-1] = 0;
            }
          }
          break;
        default:
          return;
      }
    }
  }

  uint16_t emptyCells(void) {
    uint16_t ret = 0;

    for (int r = 0; r < kRows; r++) {
      for (int c = 0; c < kCols; c++) {
        int shift = r * 4 + c;
        int empty = (board[r][c] == 0);

        ret |= (uint16_t)(!!empty) << shift;
      }
    }

    return ret;
  }

  int countBits(uint16_t bits) {
    bits = bits - ((bits >> 1) & 0x55555555);
    bits = (bits & 0x33333333) + ((bits >> 2) & 0x33333333);
    return ((bits + (bits >> 4) & 0x0F0F0F0F) * 0x01010101) >> 24;
  }

  bool checkGameOver() {
    return emptyCells() == 0;
  }

  void applyMove(Direction d) {
    slide(d);
    combine(d);
    slide(d);
  }

private:
  int board[kRows][kCols];
};

std::ostream& operator<<(std::ostream& stream, Board& b) {
  for (int r = 0; r < Board::kRows; r++) {
    for (int c = 0; c < Board::kCols; c++) {
      stream << std::setw(4) << std::right << b.val(r, c);

      if (c != Board::kCols-1)
        stream << " | ";
    }

    if (r != Board::kRows-1)
      stream << "\n";
  }

  return stream;
}

void clear_screen() {
  const char *clrscr = "\x1B[2J";
  const char *reset_cursor = "\x1B[H";
  std::cout << clrscr << reset_cursor << std::endl;
}


int main(int argc, char **argv) {
#if NCURSES
  initscr();

  cbreak();
  noecho();
  keypad(stdscr, TRUE);
#else
#endif
  
  struct termios prev{}, input{};
  int rc = 0;

  rc = tcgetattr(fileno(stdin), &input);
  if (rc < 0)
    perror("termios getattr");

  // save the input terminal settings before we mess around with them
  prev = input;

  input.c_lflag &= ~(ECHO | ICANON);

  rc = tcsetattr(fileno(stdin), TCSANOW, &input);
  if (rc < 0)
    perror("termios setattr");

  Board game{};


  bool done = false;
  do {
    clear_screen();

    std::cout << game << std::endl;

    auto direction = Board::Direction::kUnknown;
    int c = getchar();

    switch (c) {
      case 'w':
      case 'k':
        direction = Board::Direction::kUp;
        break;
      case 's':
      case 'j':
        direction = Board::Direction::kDown;
        break;
      case 'a':
      case 'h':
        direction = Board::Direction::kLeft;
        break;
      case 'd':
      case 'l':
        direction = Board::Direction::kRight;
        break;

      case 'q':
        done = true;
        break;

      // SUPER janky arrow-key processing
      case 27: { // ESC 
        c = getchar();
        if (c != '[')
          continue;

        c = getchar();
        switch (c) {
          case 'A':
            direction = Board::Direction::kUp;
            break;
          case 'B':
            direction = Board::Direction::kDown;
            break;
          case 'C':
            direction = Board::Direction::kRight;
            break;
          case 'D':
            direction = Board::Direction::kLeft;
            break;
          default:
            break;
        }
      }
    }

    game.applyMove(direction);
  } while (!done);

#if NCURSES
  endwin();
#else
  rc = tcsetattr(fileno(stdin), TCSANOW, &prev);
  if (rc < 0)
    perror("reset termios (setattr)");
#endif

  return 0;
}
