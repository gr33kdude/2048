#include "game.h"

#include <iostream>
#include <cstdint>
#include <climits>

bool Board::insertRandomValue() {
  uint16_t empty = emptyCells();
  int num_empty = countBits(empty);
  if (num_empty == 0) 
    return true;

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

  return empty_passed == insert_spot;
}

void Board::pack(int start, int stride, int arr[]) {
  for (int i = 0; i < 4; i++) {
    int r, c;
    convert(start, stride, i, r, c);
    arr[i] = board[r][c];
  }
}

void Board::unpack(int start, int stride, int arr[]) {
  for (int i = 0; i < 4; i++) {
    int r, c;
    convert(start, stride, i, r, c);
    board[r][c] = arr[i];
  }
}

void Board::operation(Direction d, Operation o) {
  for (int i = 0; i < 4; i++) {
    int arr[4] = {};
    int start = 0;
    int stride = 0;

    switch (d) {
      case Direction::kUp:
        start  = 0 + i;
        stride = 4;
        break;
      case Direction::kDown:
        start  = 12 + i;
        stride = -4;
        break;
      case Direction::kLeft:
        start  = 4 * i;
        stride = 1;
        break;
      case Direction::kRight:
        start  = 4 * i + 3;
        stride = -1;
        break;
    }

    pack(start, stride, arr);
    if (o == Operation::kSlide) {
      simple_slide(arr);
    } else if (o == Operation::kCombine) {
      simple_combine(arr);
    } else {
    }
    unpack(start, stride, arr);
  }
}

void Board::combine(Direction d) {
  operation(d, Operation::kCombine);
}

void Board::slide(Direction d) {
  operation(d, Operation::kSlide);
}


#if 0
// for each of the nonzero final tiles, scan for the next nonzero
// if equal, combine, otherwise 
void Board::slide(Direction d) {
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
#endif

uint16_t Board::emptyCells(void) {
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