#pragma once

#include "types/file.hpp"
#include "types/rank.hpp"

namespace chess {

enum square_e : uint8_t {
  a1, b1, c1, d1, e1, f1, g1, h1, 
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8
};

struct square_t final {
  using enum_t = square_e;
  using index_t = std::underlying_type_t<enum_t>;

  static constexpr index_t max = h8 + 1;

  constexpr square_t() noexcept : value() {}
  constexpr square_t(square_e value) noexcept : value(value) {}
  constexpr operator square_e() const noexcept { return square_e(value); }

  constexpr square_t(file_t file, rank_t rank) noexcept : value(file + rank * file_t::max) {}
  constexpr file_t file() const noexcept { return file_e(value % file_t::max); }
  constexpr rank_t rank() const noexcept { return rank_e(value / file_t::max); }

  constexpr square_t& operator++() noexcept { ++value; return *this; }
  constexpr square_t& operator--() noexcept { --value; return *this; }

private:
  index_t value;
};

static_assert(sizeof(square_t) == sizeof(uint8_t));
static_assert(square_t(d, _3) == d3);
static_assert(square_t(d3).file() == d);
static_assert(square_t(d3).rank() == _3);
static_assert([](){ int n = 0; for (square_t s = a1; s <= h8; ++s) { ++n; } return n; }() == square_t::max);
static_assert([](){ int n = square_t::max; for (square_t s = h8; s > a1; --s) { --n; } return n; }() == 1);

} // namespace chess

template <>
struct std::formatter<chess::square_t, char> {
    template <class ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <class FormatContext>
    auto format(chess::square_t const& value, FormatContext& ctx) const { return format_to(ctx.out(), "{}{}", value.file(), value.rank()); }
};
