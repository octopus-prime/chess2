#pragma once

#include "types/hash.hpp"
#include "types/square.hpp"
#include "types/piece.hpp"
#include "types/file.hpp"
#include <array>
#include <cstdint>

namespace chess::hash::lookup {

namespace detail {

// splitmix64: deterministic pseudo-random generator usable in consteval contexts
constexpr hash_t splitmix64(uint64_t &state) noexcept {
  uint64_t z = (state += 0x9E3779B97F4A7C15ull);
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
//   state = (state ^ (state >> 31)) * 0xD6E8FEB86659FD93ull;
  return z ^ (z >> 31);
}

} // namespace detail

constexpr hash_t piece(const piece_t piece, const square_t square) noexcept {
  using table_t = std::array<std::array<uint64_t, square_t::max>, piece_t::max>;
  static constexpr table_t table = [] static consteval noexcept {
    table_t temp{};
    uint64_t state = 0x1234567890ABCDEFull;
    for (auto &row : temp)
      for (auto &entry : row)
        entry = detail::splitmix64(state);
    return temp;
  }();
  return table[piece][square];
}

constexpr hash_t side() noexcept {
  static constexpr uint64_t value = [] static consteval noexcept {
    uint64_t state = 0xABCDEF1234567890ull;
    return detail::splitmix64(state);
  }();
  return value;
}

constexpr hash_t castle(const square_t square) noexcept {
  using table_t = std::array<uint64_t, square_t::max>;
  static constexpr table_t table = [] static consteval noexcept {
    table_t temp{};
    uint64_t state = 0x0FEDCBA987654321ull;
    for (auto &entry : temp)
      entry = detail::splitmix64(state);
    return temp;
  }();
  return table[square];
}

constexpr hash_t ep(const file_t file) noexcept {
  using table_t = std::array<uint64_t, file_t::max>;
  static constexpr table_t table = [] static consteval noexcept {
    table_t temp{};
    uint64_t state = 0x1122334455667788ull;
    for (auto &entry : temp)
      entry = detail::splitmix64(state);
    return temp;
  }();
  return table[file];
}

} // namespace chess::hash::lookup
