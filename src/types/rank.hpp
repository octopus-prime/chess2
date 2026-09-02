#pragma once

#include <cstdint>
#include <type_traits>
#include <format>

namespace chess {

enum rank_e : uint8_t {
  _1, _2, _3, _4, _5, _6, _7, _8
};

struct rank_t final {
  using enum_t = rank_e;
  using index_t = std::underlying_type_t<enum_t>;

  static constexpr index_t max = _8 + 1;

  constexpr rank_t() noexcept : value() {}
  constexpr rank_t(rank_e value) noexcept : value(value) {}
  constexpr operator rank_e() const noexcept { return rank_e(value); }

  constexpr rank_t& operator++() noexcept { ++value; return *this; }
  constexpr rank_t& operator--() noexcept { --value; return *this; }

private:
  index_t value;
};

static_assert(sizeof(rank_t) == sizeof(uint8_t));
static_assert(rank_t(_1) == _1);
static_assert([](){ int n = 0; for (rank_t r = _1; r <= _8; ++r) { ++n; } return n; }() == rank_t::max);
static_assert([](){ int n = rank_t::max; for (rank_t r = _8; r > _1; --r) { --n; } return n; }() == 1);

} // namespace chess

template <>
struct std::formatter<chess::rank_t, char> {
    template <class ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <class FormatContext>
    auto format(chess::rank_t const& value, FormatContext& ctx) const { return format_to(ctx.out(), "{:c}", '1' + value); }
};
