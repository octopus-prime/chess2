#pragma once

#include "types/side.hpp"
#include "types/type.hpp"

namespace chess {

enum piece_e : uint8_t {
  WP, WN, WB, WR, WQ, WK, 
  BP, BN, BB, BR, BQ, BK, 
  NO_PIECE
};

struct piece_t final {
  using enum_t = piece_e;
  using index_t = std::underlying_type_t<enum_t>;

  static constexpr index_t max = NO_PIECE;

  constexpr piece_t() noexcept : value(NO_PIECE) {}
  constexpr piece_t(piece_e value) noexcept : value(value) {}
  constexpr operator piece_e() const noexcept { return piece_e(value); }

  constexpr piece_t(side_t side, type_t type) noexcept : value(type + side * (type_t::max - 1)) {}
  constexpr type_t type() const noexcept { return type_e(value % (type_t::max - 1)); }
  constexpr side_t side() const noexcept { return side_e(value / (type_t::max - 1)); }

  constexpr piece_t& operator++() noexcept { ++value; return *this; }
  constexpr piece_t& operator--() noexcept { --value; return *this; }

private:
  index_t value;
};

static_assert(sizeof(piece_t) == sizeof(uint8_t));
static_assert(piece_t(WHITE, R) == WR);
static_assert(piece_t(WR).side() == WHITE);
static_assert(piece_t(WR).type() == R);
static_assert(piece_t(BLACK, K) == BK);
static_assert(piece_t(BK).side() == BLACK);
static_assert(piece_t(BK).type() == K);
static_assert([](){ int n = 0; for (piece_t p = WP; p <= BK; ++p) { ++n; } return n; }() == piece_t::max);
static_assert([](){ int n = piece_t::max; for (piece_t p = BK; p > WP; --p) { --n; } return n; }() == 1);

} // namespace chess
