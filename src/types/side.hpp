#pragma once

#include <cstdint>
#include <type_traits>

namespace chess {

enum side_e : uint8_t {
  WHITE, BLACK
};

struct side_t final {
  using enum_t = side_e;
  using index_t = std::underlying_type_t<enum_t>;

  static constexpr index_t max = BLACK + 1;

  constexpr side_t() noexcept : value() {}
  constexpr side_t(side_e value) noexcept : value(value) {}
  constexpr operator side_e() const noexcept { return side_e(value); }

  constexpr side_t operator~() const noexcept { return side_e(value ^ 1); }
  constexpr side_t operator!() const noexcept { return side_e(value ^ 1); }
  constexpr side_t& operator++() noexcept { ++value; return *this; }
  constexpr side_t& operator--() noexcept { --value; return *this; }

private:
  index_t value;
};

static_assert(sizeof(side_t) == sizeof(uint8_t));
static_assert(side_t(WHITE) == WHITE);
static_assert(!side_t(WHITE) == BLACK);
static_assert(~side_t(WHITE) == BLACK);
static_assert([](){ int n = 0; for (side_t s = WHITE; s <= BLACK; ++s) { ++n; } return n; }() == side_t::max);
static_assert([](){ int n = side_t::max; for (side_t s = BLACK; s > WHITE; --s) { --n; } return n; }() == 1);

} // namespace chess
