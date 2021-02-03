#include "game.h"

#include <cstdint>
#include <climits>
#include <iostream>
#include <iomanip>
#include <sstream>

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

bool Board::operation(Direction d, Operation o) {
  bool did_operation = false;

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
      did_operation |= simple_slide(arr);
    } else if (o == Operation::kCombine) {
      did_operation |= simple_combine(arr);
    } else if (o == Operation::kCompress) {
      did_operation |= compress(arr);
    }
    unpack(start, stride, arr);
  }

  return did_operation;
}

bool Board::combine(Direction d) {
  return operation(d, Operation::kCombine);
}

bool Board::slide(Direction d) {
  return operation(d, Operation::kSlide);
}

bool Board::compress(Direction d) {
  return operation(d, Operation::kCompress);
}

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

std::ostream& operator<<(std::ostream& stream, Board& b) {
  for (int r = 0; r < Board::kRows; r++) {
    for (int c = 0; c < Board::kCols; c++) {
      int val = b.val(r, c);

      stream << std::setw(4) << std::right;
      if (val)
        stream << val;
      else
        stream << "";

      if (c != Board::kCols-1)
        stream << " | ";
    }

    if (r != Board::kRows-1)
      stream << "\n";
  }

  return stream;
}
