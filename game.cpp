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
  }

private:
  int board[4][4];
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
