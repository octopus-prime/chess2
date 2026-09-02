#pragma once

#include <cstdint>
#include <type_traits>
#include <format>

namespace chess {

enum type_e : uint8_t {
  P, N, B, R, Q, K, 
  NO_TYPE
};

struct type_t {
  using enum_t = type_e;
  using index_t = std::underlying_type_t<enum_t>;

  static constexpr index_t max = NO_TYPE + 1;

  constexpr type_t() noexcept : value(NO_TYPE) {}
  constexpr type_t(type_e value) noexcept : value(value) {}
  constexpr operator type_e() const noexcept { return type_e(value); }

  constexpr type_t& operator++() noexcept { ++value; return *this; }
  constexpr type_t& operator--() noexcept { --value; return *this; }

private:
  index_t value;
};

static_assert(sizeof(type_t) == sizeof(uint8_t));
static_assert(type_t(P) == P);
static_assert([](){ int n = 0; for (type_t t = P; t <= NO_TYPE; ++t) { ++n; } return n; }() == type_t::max);
static_assert([](){ int n = type_t::max; for (type_t t = NO_TYPE; t > P; --t) { --n; } return n; }() == 1);

} // namespace chess

template <>
struct std::formatter<chess::type_t, char> {
    template <class ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <class FormatContext>
    auto format(chess::type_t const& value, FormatContext& ctx) const {
        static constexpr std::string_view chars[] = {"p", "n", "b", "r", "q", "k", ""};
        return format_to(ctx.out(), "{}", chars[value]);
    }
};
