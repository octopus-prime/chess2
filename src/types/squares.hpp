#pragma once

#include "types/square.hpp"
#include <bit>
#include <initializer_list>
#include <iterator>

namespace chess {

enum squares_e : uint64_t {
  NO_SQUARES = 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ull,
  ALL_SQUARES = 0b11111111'11111111'11111111'11111111'11111111'11111111'11111111'11111111ull
};

struct squares_t final {
  using enum_t = squares_e;
  using mask_t = std::underlying_type_t<enum_t>;

  struct iterator_t {
    constexpr iterator_t(mask_t mask) noexcept : mask{mask} {}
    constexpr square_t operator*() const noexcept { return square_e(std::countr_zero(mask)); }
    constexpr iterator_t &operator++() noexcept { mask &= (mask - 1); return *this; }
    friend constexpr bool operator==(const iterator_t &it, std::default_sentinel_t) noexcept { return it.mask == NO_SQUARES; }

  private:
    mask_t mask;
  };

  constexpr squares_t(squares_e squares = NO_SQUARES) noexcept : value{squares} {}
  constexpr operator squares_e() const noexcept { return squares_e(value); }

  constexpr squares_t(std::initializer_list<square_t> squares) noexcept : value{} {
    constexpr mask_t mask = 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000001ull;
    for (square_t square : squares) value |= mask << square;
  }
  constexpr squares_t(std::initializer_list<file_t> files) noexcept : value{} {
    constexpr mask_t mask = 0b00000001'00000001'00000001'00000001'00000001'00000001'00000001'00000001ull;
    for (file_t file : files) value |= mask << file;
  }
  constexpr squares_t(std::initializer_list<rank_t> ranks) noexcept : value{} {
    constexpr mask_t mask = 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'11111111ull;
    for (rank_t rank : ranks) value |= mask << (rank * file_t::max);
  }

  constexpr void set(const square_t square) noexcept { value |= squares_t{square}; }
  constexpr void set(const squares_t squares) noexcept { value |= squares; }
  constexpr void reset(const square_t square) noexcept { value &= ~squares_t{square}; }
  constexpr void reset(const squares_t squares) noexcept { value &= ~squares; }
  constexpr void flip(const square_t square) noexcept { value ^= squares_t{square}; }
  constexpr void flip(const squares_t squares) noexcept { value ^= squares; }
  constexpr bool test(const square_t square) const noexcept { return (value & squares_t{square}) != NO_SQUARES; }
  constexpr bool operator[](const square_t square) const noexcept { return test(square); }

  constexpr bool empty() const noexcept { return value == NO_SQUARES; }
  constexpr size_t size() const noexcept { return size_t(std::popcount(value)); }
  constexpr square_t front() const noexcept { return square_e(std::countr_zero(value)); }
  constexpr square_t back() const noexcept { return square_e(63 - std::countl_zero(value)); }
  constexpr iterator_t begin() const noexcept { return {value}; }
  constexpr std::default_sentinel_t end() const noexcept { return {}; }

  constexpr squares_t& operator&=(squares_t other) noexcept { value &= other.value; return *this; }
  constexpr squares_t& operator|=(squares_t other) noexcept { value |= other.value; return *this; }
  constexpr squares_t& operator^=(squares_t other) noexcept { value ^= other.value; return *this; }

  friend constexpr squares_t operator~(squares_t squares) noexcept { return squares_e(~squares.value); }
  friend constexpr squares_t operator&(squares_t lhs, squares_t rhs) noexcept { return squares_e(lhs.value & rhs.value); }
  friend constexpr squares_t operator|(squares_t lhs, squares_t rhs) noexcept { return squares_e(lhs.value | rhs.value); }
  friend constexpr squares_t operator^(squares_t lhs, squares_t rhs) noexcept { return squares_e(lhs.value ^ rhs.value); }

  friend constexpr squares_t operator<<(squares_t squares, int shift) noexcept { return squares_e(squares.value << shift); }
  friend constexpr squares_t operator>>(squares_t squares, int shift) noexcept { return squares_e(squares.value >> shift); }

//   friend constexpr squares_t pext(squares_t squares, squares_t mask) noexcept {
//     return squares_e(_pext_u64(squares.value, mask.value));
//   }

//   friend constexpr squares_t pdep(squares_t squares, squares_t mask) noexcept {
//     return squares_e(_pdep_u64(squares.value, mask.value));
//   }

private:
  mask_t value;
};

static_assert(sizeof(squares_t) == sizeof(uint64_t));
static_assert((squares_t{a1, b2} & squares_t{a1, h8}) == squares_t{a1});
static_assert((squares_t{a1, b2} | squares_t{a1, h8}) == squares_t{a1, b2, h8});
static_assert((squares_t{a1, b2} ^ squares_t{a1, h8}) == squares_t{b2, h8});
static_assert(squares_t{a1, b2}.empty() == false);
static_assert(squares_t{a1, b2}.size() == 2);
static_assert(squares_t{a1, b2}.test(a1) == true);
static_assert(squares_t{a1, b2}.front() == a1);
static_assert(squares_t{a1, b2}.back() == b2);
static_assert(*squares_t{a1, b2}.begin() == a1);
static_assert((squares_t{c, e} & squares_t{_3, _6}) == squares_t{c3, e3, c6, e6});

} // namespace chess
