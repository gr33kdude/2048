#include <iostream>
#include <sstream>
#include <iomanip>

class Board {
public:
  constexpr static int kRows = 4;
  constexpr static int kCols = 4;

  enum class Direction : int {
    kUp,
    kDown,
    kLeft,
    kRight,
  };

  Board() {
    for (int r = 0; r < kRows; r++) {
      for (int c = 0; c < kCols; c++) {
        board[r][c] = 0;
      }
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
    for (int r = 0; r < kRows; r++) {
      for (int c = 0; c < kCols; c++) {
        if (empty_passed == insert_spot) {
          board[r][c] = insert_value; 
          break;
        }

        if (board[r][c] == 0) {
          empty_passed++;
        }
      }
    }

    if (empty_passed != insert_value)
      std::cout << "WAT" << std::endl;
  }

  uint16_t emptyCells(void) {
    uint16_t ret = 0;

    for (int r = 0; r < kRows; r++) {
      for (int c = 0; c < kCols; c++) {
        int shift = r * 4 + c;
        int empty = board[r][c] > 0;

        ret |= (!!empty) << shift;
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

int main(int argc, char **argv) {
  Board game{};

  std::cout << game << std::endl;
  return 0;
}
