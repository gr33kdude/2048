#include "game.h"

#include <iostream>
#include <sstream>
#include <iomanip>

void clear_screen() {
  const char *clrscr = "\x1B[2J";
  const char *reset_cursor = "\x1B[H";
  std::cout << clrscr << reset_cursor << std::endl;
}

int main(int argc, char **argv) {
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

  rc = tcsetattr(fileno(stdin), TCSANOW, &prev);
  if (rc < 0)
    perror("reset termios (setattr)");

  return 0;
}
