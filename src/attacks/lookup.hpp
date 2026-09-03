#pragma once

#include "attacks/generator.hpp"

namespace chess::attacks::lookup {

constexpr squares_t king(const square_t king) noexcept {
  using table_t = std::array<squares_t, square_t::max>;
  static_assert(sizeof(table_t) == 512);
  static constexpr table_t table = [] static consteval noexcept {
    table_t temp{};
    for (square_t square = a1; square <= h8; ++square)
      temp[square] = generator::leapers({square}, {});
    return temp;
  }();
  return table[king];
}

constexpr squares_t knight(const square_t knight) noexcept {
  using table_t = std::array<squares_t, square_t::max>;
  static_assert(sizeof(table_t) == 512);
  static constexpr table_t table = [] static consteval noexcept {
    table_t temp{};
    for (square_t square = a1; square <= h8; ++square)
      temp[square] = generator::leapers({}, {square});
    return temp;
  }();
  return table[knight];
}

constexpr squares_t rook(const square_t rook, const squares_t occupied) noexcept {
  using table_t = std::array<std::array<uint8_t, 256>, 8>;
  static_assert(sizeof(table_t) == 2048);
  static constexpr table_t table = [] static consteval noexcept {
    table_t temp{};
    for (file_t file = a; file <= h; ++file) {
      constexpr squares_t all{_1};
      const squares_t squares{square_t(file, _1)};
      for (size_t index = 0; index < 256; ++index) {
        const squares_t blockers = squares_e(_pdep_u64(index, all));
        const squares_t attacks = generator::sliders(squares, {}, blockers);
        temp[file][index] = uint8_t(_pext_u64(attacks, all));
      }
    }
    return temp;
  }();
  const squares_t rank_mask{rook.rank()};
  const squares_t file_mask{rook.file()};
  const squares_t rank_attacks = squares_e(_pdep_u64(table[rook.file()][_pext_u64(occupied, rank_mask)], rank_mask));
  const squares_t file_attacks = squares_e(_pdep_u64(table[rook.rank()][_pext_u64(occupied, file_mask)], file_mask));
  return rank_attacks | file_attacks;
}

constexpr squares_t bishop(const square_t bishop, const squares_t occupied) noexcept {
    struct meta_t {
        uint64_t occ;
        uint64_t all;
        size_t offset;
    };
  struct table_t {
    std::array<meta_t, square_t::max> meta{};
    std::array<uint16_t, 5248> data{};
  };
  static_assert(sizeof(table_t) == 12032);
  static constexpr table_t table = [] static consteval noexcept {
    constexpr auto relevant_occupancy = [](const square_t square, const squares_t occupied) static consteval noexcept {
        uint64_t mask = occupied;
        if (square.rank() > _1) mask &= ~squares_t{_1};
        if (square.rank() < _8) mask &= ~squares_t{_8};
        if (square.file() > a) mask &= ~squares_t{a};
        if (square.file() < h) mask &= ~squares_t{h};
        return squares_e(mask);
    };
    table_t temp{};
    size_t offset{};
    for (square_t square = a1; square <= h8; ++square) {
      const squares_t squares{square};
      const squares_t all = generator::sliders({}, squares, {});
      const squares_t occ = relevant_occupancy(square, all);
      temp.meta[square] = {occ, all, offset};
      for (size_t index = 0; index < (1u << occ.size()); ++index, ++offset) {
        const squares_t blockers = squares_e(_pdep_u64(index, occ));
        const squares_t attacks = generator::sliders({}, squares, blockers);
        temp.data[offset] = uint16_t(_pext_u64(attacks, all));
      }
    }
    return temp;
  }();
  const meta_t &m = table.meta[bishop];
  return squares_e(_pdep_u64(table.data[m.offset + _pext_u64(occupied, m.occ)], m.all));
}

constexpr squares_t queen(const square_t queen, const squares_t occupied) noexcept {
  return rook(queen, occupied) | bishop(queen, occupied);
}

constexpr squares_t pawn(const square_t pawn, const side_t side) noexcept {
  using table_t = std::array<std::array<squares_t, square_t::max>, side_t::max>;
  static_assert(sizeof(table_t) == 1024);
  static constexpr table_t table = [] static consteval noexcept {
    table_t temp{};
    for (side_t side = WHITE; side <= BLACK; ++side)
      for (square_t square = a1; square <= h8; ++square)
        temp[side][square] = generator::pawns({square}, side);
    return temp;
  }();
  return table[side][pawn];
}

// squares strictly between two rank/file-aligned squares
constexpr squares_t between_straight(square_t from, square_t to) noexcept {
  return rook(from, squares_t{to}) & rook(to, squares_t{from});
}

// squares strictly between two diagonally-aligned squares
constexpr squares_t between_diagonal(square_t from, square_t to) noexcept {
  return bishop(from, squares_t{to}) & bishop(to, squares_t{from});
}

static_assert(king(e4) == squares_t{d3, e3, f3, d4, f4, d5, e5, f5});
static_assert(knight(e4) == squares_t{c3, d2, f2, g3, g5, f6, d6, c5});
static_assert(rook(e4, squares_t{}) == squares_t{e1, e2, e3, e5, e6, e7, e8, a4, b4, c4, d4, f4, g4, h4});
static_assert(bishop(e4, squares_t{}) == squares_t{b1, c2, d3, f5, g6, h7, a8, b7, c6, d5, f3, g2, h1});
static_assert(queen(e4, squares_t{}) == squares_t{b1, c2, d3, f5, g6, h7, a8, b7, c6, d5, f3, g2, h1, e1, e2, e3, e5, e6, e7, e8, a4, b4, c4, d4, f4, g4, h4}); 
static_assert(pawn(e4, WHITE) == squares_t{d5, f5});
static_assert(pawn(e4, BLACK) == squares_t{d3, f3});
static_assert(between_straight(e4, e8) == squares_t{e5, e6, e7});
static_assert(between_straight(e4, a4) == squares_t{b4, c4, d4});
static_assert(between_diagonal(e4, h7) == squares_t{f5, g6});
static_assert(between_diagonal(e4, a8) == squares_t{d5, c6, b7});
static_assert(between_diagonal(a1, h8) == squares_t{b2, c3, d4, e5, f6, g7});

} // namespace chess::attacks::lookup
