#pragma once

#include "type.hpp"

namespace chess {

enum score_e : int16_t {MIN = -32000, MAX = 32000, DRAW = 0, WIN = 30000, LOSS = -30000, PAWN = 100, KNIGHT = 300, BISHOP = 300, ROOK = 500, QUEEN = 900, KING = 20000, TIME_OUT = -32100};
struct score_t final {
    using enum_t = score_e;
    using value_t = std::underlying_type_t<enum_t>;

    constexpr score_t(const score_e v = DRAW) noexcept : value(v) {}
    constexpr score_t(const type_t t) noexcept {
        constexpr std::array<value_t, type_t::max> scores {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, DRAW};
        value = scores[size_t(t)];
    }
    constexpr operator score_e() const noexcept { return score_e(value); }
    constexpr bool is_win() const noexcept { return value >= WIN; }
    constexpr bool is_loss() const noexcept { return value <= LOSS; }
    constexpr bool is_decisive() const noexcept { return is_win() || is_loss(); }
    constexpr bool is_draw() const noexcept { return value == DRAW; }
    constexpr score_t operator-() const noexcept { return score_e(-value); }

private:
    value_t value;
};

static_assert(sizeof(score_t) == sizeof(int16_t));
static_assert(score_t(MIN).is_loss());
static_assert(score_t(MAX).is_win());
static_assert(score_t(DRAW).is_draw());
static_assert(score_t(WIN).is_win());
static_assert(score_t(LOSS).is_loss());
static_assert(score_t(MIN).is_decisive());
static_assert(score_t(MAX).is_decisive());
static_assert(score_t(WIN).is_decisive());
static_assert(score_t(LOSS).is_decisive());

} // namespace chess

template <>
struct std::formatter<chess::score_t, char> {
    template <class ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <class FormatContext>
    auto format(chess::score_t const& value, FormatContext& ctx) const {
        if (value.is_decisive()) {
            const int dist = value.is_win() ? chess::MAX - value + 1 : chess::MIN - value;
            return format_to(ctx.out(), "mate {:+}", dist / 2);
        }
        else
            return format_to(ctx.out(), "cp {:+}", int(value));
    }
};
