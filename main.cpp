#include "game.h"

#include <iostream>
#include <sstream>
#include <iomanip>

#define NCURSES 0

#if NCURSES
# include <curses.h>
#else
# include <termios.h>
#endif

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
  } while (!done || !game.checkGameOver());

#if NCURSES
  endwin();
#else
  rc = tcsetattr(fileno(stdin), TCSANOW, &prev);
  if (rc < 0)
    perror("reset termios (setattr)");
#endif

  return 0;
}
