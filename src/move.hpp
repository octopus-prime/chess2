#pragma once

#include "types/square.hpp"
#include "types/type.hpp"
#include <array>

namespace chess {

struct move_t final {
  constexpr static int FROM_SQ_SHIFT = 0;
  constexpr static int TO_SQ_SHIFT   = 6;
  constexpr static int PROMOTION_SHIFT = 12;

  constexpr move_t() noexcept = default;
  constexpr move_t(const square_t from, const square_t to, const type_t promotion = NO_TYPE) noexcept : _from{from}, _to{to}, _promotion{promotion} {}

  constexpr operator uint16_t() const noexcept { return uint16_t(_from | (_to << 6) | (_promotion << 12)); }

  constexpr square_t from() const noexcept { return square_e(_from); }
  constexpr square_t to() const noexcept { return square_e(_to); }
  constexpr type_t promotion() const noexcept { return type_e(_promotion); }

private:
  uint16_t _from : 6;
  uint16_t _to : 6;
  uint16_t _promotion : 4;
};

static_assert(sizeof(move_t) == sizeof(uint16_t));
static_assert(move_t(a1, b2, Q) == 16960);
static_assert(move_t(a1, b2, Q).from() == a1);
static_assert(move_t(a1, b2, Q).to() == b2);
static_assert(move_t(a1, b2, Q).promotion() == Q);

using moves_t = std::array<move_t, 256>;

} // namespace chess

template <>
struct std::formatter<chess::move_t, char> {
    template <class ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <class FormatContext>
    auto format(chess::move_t const& value, FormatContext& ctx) const {
        return format_to(ctx.out(), "{}{}{}", value.from(), value.to(), value.promotion());
    }
};
