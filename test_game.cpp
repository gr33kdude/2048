#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "game.h"

#define DIM(a) (sizeof(a) / sizeof(*(a)))

TEST_CASE( "Combine" ) {
  Board game{};

  SECTION( "Test 1" ) {
    int in_arr[] = { 0, 0, 0, 0 };
    int out_arr[] = { 0, 0, 0, 0 };

    Board::simple_combine(in_arr);

    REQUIRE(!memcmp(in_arr, out_arr, sizeof(in_arr)));
  }
  
  SECTION( "Test 2" ) {
    int in_arr[] = { 2, 0, 0, 0 };
    int out_arr[] = { 2, 0, 0, 0 };

    Board::simple_combine(in_arr);

    REQUIRE(!memcmp(in_arr, out_arr, sizeof(in_arr)));
  }

  SECTION( "Test 3" ) {
    int in_arr[] = { 2, 2, 0, 0 };
    int out_arr[] = { 4, 0, 0, 0 };

    Board::simple_combine(in_arr);

    REQUIRE(!memcmp(in_arr, out_arr, sizeof(in_arr)));
  }
  
  SECTION( "Test 4" ) {
    int in_arr[] = { 2, 0, 2, 0 };
    int out_arr[] = { 4, 0, 0, 0 };

    Board::simple_slide(in_arr);
    Board::simple_combine(in_arr);

    REQUIRE(!memcmp(in_arr, out_arr, sizeof(in_arr)));
  }

  SECTION( "Test 5" ) {
    int in_arr[] = { 4, 2, 2, 0 };
    int out_arr[] = { 4, 4, 0, 0 };

    Board::simple_combine(in_arr);

    REQUIRE(!memcmp(in_arr, out_arr, sizeof(in_arr)));
  }

  SECTION( "Test 6" ) {
    int in_arr[] = { 8, 4, 4, 0 };
    int out_arr[] = { 8, 8, 0, 0 };

    Board::simple_combine(in_arr);

    REQUIRE(!memcmp(in_arr, out_arr, sizeof(in_arr)));
  }
}

TEST_CASE( "Slide" ) {
  Board game{};

  SECTION( "Empty" ) {
    int in_arr[] = { 0, 0, 0, 0 };
    int out_arr[] = { 0, 0, 0, 0 };

    Board::simple_slide(in_arr);

    REQUIRE(!memcmp(in_arr, out_arr, sizeof(in_arr)));
  }
  
  SECTION( "Single" ) {
    int in_arr[4] = {};
    int out_arr[] = { 2, 0, 0, 0 };

    int test_arr[][4] = {
      { 2, 0, 0, 0 },
      { 0, 2, 0, 0 },
      { 0, 0, 2, 0 },
      { 0, 0, 0, 2 },
    };
    size_t N = DIM(test_arr);

    for (int i = 0; i < N; i++) {
      memcpy(in_arr, &test_arr[i], sizeof(in_arr));
      Board::simple_slide(in_arr);

      REQUIRE(!memcmp(in_arr, out_arr, sizeof(in_arr)));
    }
  }

  SECTION( "Double" ) {
    int in_arr[4] = {};
    int out_arr[] = { 2, 2, 0, 0 };

    int test_arr[][4] = {
      { 2, 2, 0, 0 },
      { 2, 0, 2, 0 },
      { 2, 0, 0, 2 },
      { 0, 2, 2, 0 },
      { 0, 2, 0, 2 },
      { 0, 0, 2, 2 },
    };
    size_t N = DIM(test_arr);

    for (int i = 0; i < N; i++) {
      memcpy(in_arr, &test_arr[i], sizeof(in_arr));
      Board::simple_slide(in_arr);

      REQUIRE(!memcmp(in_arr, out_arr, sizeof(in_arr)));
    }
  }

  SECTION( "Double 2" ) {
    int in_arr[4] = {};
    int out_arr[] = { 4, 2, 0, 0 };

    int test_arr[][4] = {
      { 4, 2, 0, 0 },
      { 4, 0, 2, 0 },
      { 4, 0, 0, 2 },
      { 0, 4, 2, 0 },
      { 0, 4, 0, 2 },
      { 0, 0, 4, 2 },
    };
    size_t N = DIM(test_arr);

    for (int i = 0; i < N; i++) {
      memcpy(in_arr, &test_arr[i], sizeof(in_arr));
      Board::simple_slide(in_arr);

      REQUIRE(!memcmp(in_arr, out_arr, sizeof(in_arr)));
    }
  }
}

TEST_CASE( "Convert" ) {
  struct test_case {
    int start;
    int stride;
    int i;
    int r;
    int c;
  } test_cases[] = {
    { 0, 1, 0, 0, 0 },
    { 0, 1, 1, 0, 1 },
    { 0, 1, 2, 0, 2 },
    { 0, 1, 3, 0, 3 },

    { 4, 1, 0, 1, 0 },
    { 4, 1, 1, 1, 1 },
    { 4, 1, 2, 1, 2 },
    { 4, 1, 3, 1, 3 },

    { 0, 4, 0, 0, 0 },
    { 0, 4, 1, 1, 0 },
    { 0, 4, 2, 2, 0 },
    { 0, 4, 3, 3, 0 },

    // DOWN
    { 12, -4, 0, 3, 0 },
    { 12, -4, 1, 2, 0 },
    { 12, -4, 2, 1, 0 },
    { 12, -4, 3, 0, 0 },

    { 13, -4, 0, 3, 1 },
    { 13, -4, 1, 2, 1 },
    { 13, -4, 2, 1, 1 },
    { 13, -4, 3, 0, 1 },

    { 14, -4, 0, 3, 2 },
    { 14, -4, 1, 2, 2 },
    { 14, -4, 2, 1, 2 },
    { 14, -4, 3, 0, 2 },

    { 15, -4, 0, 3, 3 },
    { 15, -4, 1, 2, 3 },
    { 15, -4, 2, 1, 3 },
    { 15, -4, 3, 0, 3 },
  };
  
  for (int i = 0; i < DIM(test_cases); i++) {
    struct test_case *tc = &test_cases[i];

    int r, c;
    Board::convert(tc->start, tc->stride, tc->i, r, c);
    REQUIRE(r == tc->r);
    REQUIRE(c == tc->c);
  }
}
