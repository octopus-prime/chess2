#pragma once

#include <cstdint>
#include <type_traits>
#include <format>

namespace chess {

enum file_e : uint8_t {
  a, b, c, d, e, f, g, h
};

struct file_t final {
  using enum_t = file_e;
  using index_t = std::underlying_type_t<enum_t>;

  static constexpr index_t max = h + 1;

  constexpr file_t() noexcept : value() {}
  constexpr file_t(file_e value) noexcept : value(value) {}
  constexpr operator file_e() const noexcept { return file_e(value); }

  constexpr file_t& operator++() noexcept { ++value; return *this; }
  constexpr file_t& operator--() noexcept { --value; return *this; }

private:
  index_t value;
};

static_assert(sizeof(file_t) == sizeof(uint8_t));
static_assert(file_t(a) == a);
static_assert(file_t(a) < b);
static_assert([](){ int n = 0; for (file_t f = a; f <= h; ++f) { ++n; } return n; }() == file_t::max);
static_assert([](){ int n = file_t::max; for (file_t f = h; f > a; --f) { --n; } return n; }() == 1);

} // namespace chess

template <>
struct std::formatter<chess::file_t, char> {
    template <class ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <class FormatContext>
    auto format(chess::file_t const& value, FormatContext& ctx) const { return format_to(ctx.out(), "{:c}", 'a' + value); }
};
