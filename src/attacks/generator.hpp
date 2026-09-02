#pragma once

#include <immintrin.h>

#include "types/side.hpp"
#include "types/squares.hpp"

namespace chess::attacks::generator {

constexpr squares_t leapers(const squares_t kings, const squares_t knights) noexcept {
  static constexpr __v8du shift{1,  8,  7,  9, 10, 17, 15, 6};
  static constexpr __v8du exclude_left{
      squares_t{a},    squares_t(),  squares_t{h}, squares_t{a},
      squares_t{a, b}, squares_t{a}, squares_t{h}, squares_t{g, h}};
  static constexpr __v8du exclude_right{
      squares_t{h},    squares_t(),  squares_t{a}, squares_t{h},
      squares_t{g, h}, squares_t{h}, squares_t{a}, squares_t{a, b}};
  const __v8du mask{kings,   kings,   kings,   kings,
                    knights, knights, knights, knights};
  const __v8du left = (mask << shift) & ~exclude_left;
  const __v8du right = (mask >> shift) & ~exclude_right;
  return squares_e(__builtin_reduce_or(left | right));
}

constexpr squares_t sliders(const squares_t rooks, const squares_t bishops, const squares_t occupied) noexcept {
  static constexpr __v4du shift{1, 8, 7, 9};
  static constexpr __v4du exclude_left{squares_t{a}, squares_t(), squares_t{h}, squares_t{a}};
  static constexpr __v4du exclude_right{squares_t{h}, squares_t(), squares_t{a}, squares_t{h}};
  const __v4du mask{rooks, rooks, bishops, bishops};

  __v4du left(mask);
  __v4du right(mask);

  __v4du board(uint64_t(~occupied) & ~exclude_left);
  left |= board & (left << shift);
  board &= (board << shift);
  left |= board & (left << (shift * 2));
  board &= (board << (shift * 2));
  left |= board & (left << (shift * 4));
  left = (left << shift) & ~exclude_left;

  board = uint64_t(~occupied) & ~exclude_right;
  right |= board & (right >> shift);
  board &= (board >> shift);
  right |= board & (right >> (shift * 2));
  board &= (board >> (shift * 2));
  right |= board & (right >> (shift * 4));
  right = (right >> shift) & ~exclude_right;

  return squares_e(__builtin_reduce_or(left | right));
}

constexpr squares_t pawns(const squares_t pawns, const side_t side) noexcept {
  static constexpr __v2du shift{9, 7};
  static constexpr __v2du exclude_left{squares_t{a}, squares_t{h}};
  static constexpr __v2du exclude_right{squares_t{h}, squares_t{a}};
  const __v2du mask ={pawns, pawns};
  const __v2du result = side == WHITE ? (mask << shift) & ~exclude_left
                                      : (mask >> shift) & ~exclude_right;
  return squares_e(__builtin_reduce_or(result));
}

static_assert(leapers({e1}, {}) == squares_t{d1, f1, d2, e2, f2});
static_assert(leapers({e8}, {}) == squares_t{d8, f8, d7, e7, f7});
static_assert(leapers({}, {b1, g1}) == squares_t{a3, c3, d2, e2, f3, h3});
static_assert(leapers({}, {b8, g8}) == squares_t{a6, c6, d7, e7, f6, h6});
static_assert(leapers({e1}, {b1, g1}) == (squares_t{d1, f1, d2, e2, f2, a3, c3, d2, e2, f3, h3}));
static_assert(leapers({e8}, {b8, g8}) == (squares_t{d8, f8, d7, e7, f7, a6, c6, d7, e7, f6, h6}));
static_assert(sliders({e4}, {}, {}) == squares_t{a4, b4, c4, d4, f4, g4, h4, e1, e2, e3, e5, e6, e7, e8});
static_assert(sliders({}, {e4}, {}) == squares_t{b1, c2, d3, f5, g6, h7, a8, b7, c6, d5, f3, g2, h1});
static_assert(pawns({e2}, WHITE) == squares_t{d3, f3});
static_assert(pawns({e7}, BLACK) == squares_t{d6, f6});

} // namespace chess::attacks::generator
