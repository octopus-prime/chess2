#pragma once

#include "types/side.hpp"
#include "types/square.hpp"
#include "types/piece.hpp"
#include "types/squares.hpp"
#include "types/score.hpp"
#include "attacks/lookup.hpp"
#include "hash/lookup.hpp"
#include "move.hpp"
#include <array>
#include <cstdint>
#include <ranges>
#include <span>
#include <algorithm>

namespace chess {

constexpr size_t MAX_PLY = 256;

struct state_t {
  piece_t captured;
  square_t captured_square;
  uint8_t half_moves;
  squares_t special; // en_passant + castling
  hash_t hash;
};

static_assert(sizeof(state_t) == 24);

struct position_t final {
  constexpr position_t() noexcept : position_t("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {}

  constexpr position_t(const std::string_view fen) noexcept {
    constexpr auto castle_lookup = [](char ch) static -> squares_t {
        switch (ch) {
            case 'K': return squares_t{h1}; case 'k': return squares_t{h8};
            case 'Q': return squares_t{a1}; case 'q': return squares_t{a8};
            default: return squares_t{};
        }
    };
    constexpr auto piece_lookup = [](char ch) static -> piece_t {
        switch (ch) {
            case 'P': return WP; case 'p': return BP;
            case 'N': return WN; case 'n': return BN;
            case 'B': return WB; case 'b': return BB;
            case 'R': return WR; case 'r': return BR;
            case 'Q': return WQ; case 'q': return BQ;
            case 'K': return WK; case 'k': return BK;
            default:  return piece_t{};
        }
    };

    // state_t new_state{};

    auto fen_parts = fen | std::views::split(' ');
    auto fen_part = fen_parts.begin();

    std::string_view board_part {*fen_part++};
    for (auto [rank_index, rank_part] : std::views::enumerate(board_part | std::views::split('/'))) {
        int file_index = 0;
        for (auto ch : rank_part) {
            // if (std::isdigit(ch)) {
            if (ch >= '0' && ch <= '9') {
                file_index += ch - '0';
            } else {
                square_t square{static_cast<file_e>(file_index++), static_cast<rank_e>(7 - rank_index)};
                // piece_t piece{std::string_view{&ch, 1}};
                piece_t piece{piece_lookup(ch)};
                piece_at_square[square] = piece;
                // occupied_by_type[piece.type()] |= squares_t{square};
                // occupied_by_side[piece.side()] |= squares_t{square};
                occupied_by_type[piece.type()].set(square);
                occupied_by_side[piece.side()].set(square);
                // material[piece.side()] += type_values[piece.type()];
                // new_state.hash ^= hashes::hash(piece, square);
                // if (piece.type() == PAWN)
                //     new_state.pawn_hash ^= hashes::hash(piece, square);
                // else if (piece.type() == KNIGHT || piece.type() == BISHOP)
                //     new_state.minor_hash ^= hashes::hash(piece, square);
                // else if (piece.type() == ROOK || piece.type() == QUEEN)
                //     new_state.major_hash ^= hashes::hash(piece, square);
            }
        }
    }

    std::string_view side_part {*fen_part++};
    if (side_part[0] == 'w') {
        side_to_move = WHITE;
    } else {
        side_to_move = BLACK;
        // new_state.hash ^= hashes::side();
    }

    std::string_view castle_part {*fen_part++};
    if (castle_part[0] != '-') {
        for (auto ch : castle_part) {
            // new_state.castle |= castle_lookup(ch);
            castling_rights |= castle_lookup(ch);
        }
        // new_state.hash ^= hashes::castle(new_state.castle);
    }

    std::string_view en_passant_part {*fen_part++};
    if (en_passant_part[0] != '-')
        en_passant = squares_t{square_t{file_e(en_passant_part[0] - 'a'), rank_e(en_passant_part[1] - '1')}};

    if (fen_part != fen_parts.end()) {
        std::string_view half_moves_part {*fen_part++};
        std::from_chars(&*half_moves_part.begin(), &*half_moves_part.end(), half_moves);
        // std::from_chars(&*half_moves_part.begin(), &*half_moves_part.end(), new_state.half_move);
    }

    // if (fen_part != fen_parts.end()) {
    //     std::string_view full_move_part {*fen_part++};
    //     std::from_chars(&*full_move_part.begin(), &*full_move_part.end(), full_move);
    // }

    // auto king_square = by(side, KING).front();
    // new_state.checkers = attackers(king_square) & by(~side);

    // for (side_e side : {WHITE, BLACK}) {
    //     auto [snipers, blockers] = find_snipers_and_blockers(side);
    //     new_state.snipers[side] = snipers;
    //     new_state.blockers[side] = blockers;
    // }

    // new_state.repetition = 1;

    // new_state.captured = NO_PIECE;
    // new_state.last_move = move_t{};

    // states.push_back(new_state);

    current_hash = recompute_hash();
  }

  constexpr piece_t at(const square_t square) const noexcept { return piece_at_square[square]; }
  constexpr squares_t by() const noexcept { return by(WHITE) | by(BLACK); }
  constexpr squares_t by(const side_t side) const noexcept { return occupied_by_side[side]; }
  constexpr squares_t by(const type_t type) const noexcept { return occupied_by_type[type]; }
  constexpr squares_t by(const piece_t piece) const noexcept { return by(piece.side()) & by(piece.type()); }
  constexpr squares_t by(const side_t side, const type_t type) const noexcept { return by(side) & by(type); }
  constexpr squares_t by(const type_t type1, const type_t type2) const noexcept { return by(type1) | by(type2); }
  constexpr squares_t by(const side_t side, const type_t type1, const type_t type2) const noexcept { return by(side) & (by(type1, type2)); }
  constexpr squares_t ep() const noexcept { return en_passant; }
  constexpr squares_t castle() const noexcept { return castling_rights; }
  constexpr side_t side() const noexcept { return side_to_move; }
  constexpr hash_t hash() const noexcept { return current_hash; }

  constexpr squares_t checkers(side_t side) const noexcept {
    const square_t ksq = by(side, K).front();
    return attacked(ksq, side);
  }

  // whether the enemy of `defender` attacks `square` on the current board
  constexpr squares_t attacked(const square_t square, const side_t defender) const noexcept {
    using namespace attacks::lookup;
    const side_t attacker = !defender;
    return (king(square) & by(attacker, K)) |
             (knight(square) & by(attacker, N)) |
             (rook(square, by()) & by(attacker, R, Q)) |
             (bishop(square, by()) & by(attacker, B, Q)) |
             (pawn(square, defender) & by(attacker, P));
  }

  // own pieces that would expose the king to a slider attack if moved
  constexpr squares_t pinned(side_t side) const noexcept {
    using namespace attacks::lookup;

    const square_t king = by(side, K).front();
    const squares_t own = by(side);
    const squares_t occ = by();
    squares_t result{};

    for (const square_t enemy : rook(king, squares_t{}) & by(!side, R, Q)) {
      const squares_t blockers = between_straight(king, enemy) & occ;
      if (blockers.size() == 1)
        result |= blockers & own;
    }

    for (const square_t enemy : bishop(king, squares_t{}) & by(!side, B, Q)) {
      const squares_t blockers = between_diagonal(king, enemy) & occ;
      if (blockers.size() == 1)
        result |= blockers & own;
    }

    return result;
  }

  // Stockfish-style static-exchange evaluation: true if the exchange on move.to() nets >= threshold
  constexpr bool see_ge(const move_t move, const score_t threshold) const noexcept {
    using namespace attacks::lookup;

    const square_t from = move.from();
    const square_t to = move.to();
    const piece_t moved = at(from);
    const piece_t captured_piece = at(to);
    const bool is_ep = moved.type() == P && !ep().empty() && squares_t{to} == ep();
    const type_t promotion = move.promotion();

    // NO_PIECE.type() aliases to P, so an empty target must be checked explicitly
    const type_t captured_type = is_ep ? type_t(P) : (captured_piece == NO_PIECE ? type_t(NO_TYPE) : captured_piece.type());

    int swap = int(score_t(captured_type)) - int(threshold);
    if (promotion != NO_TYPE)
      swap += int(score_t(promotion)) - int(score_t(P));
    if (swap < 0)
      return false;

    const type_t moved_type = promotion != NO_TYPE ? promotion : moved.type();
    swap = int(score_t(moved_type)) - swap;
    if (swap <= 0)
      return true;

    squares_t occupied = by();
    occupied.reset(from);
    occupied.reset(to);
    if (is_ep)
      occupied.reset(square_e(int(to) + (moved.side() == WHITE ? -8 : 8)));

    squares_t attackers = (king(to) & by(K)) | (knight(to) & by(N)) |
                          (rook(to, occupied) & by(R, Q)) | (bishop(to, occupied) & by(B, Q)) |
                          (pawn(to, WHITE) & by(BLACK, P)) | (pawn(to, BLACK) & by(WHITE, P));
    attackers &= occupied;

    side_t stm = moved.side();
    int res = 1;

    while (true) {
      stm = ~stm;
      attackers &= occupied;

      const squares_t stm_attackers = attackers & by(stm);
      if (stm_attackers.empty())
        break;

      res ^= 1;

      squares_t bb;
      if (!(bb = stm_attackers & by(P)).empty()) {
        if ((swap = int(score_t(P)) - swap) < res)
          break;
        occupied.reset(bb.front());
        attackers |= bishop(to, occupied) & by(B, Q);
      } else if (!(bb = stm_attackers & by(N)).empty()) {
        if ((swap = int(score_t(N)) - swap) < res)
          break;
        occupied.reset(bb.front());
      } else if (!(bb = stm_attackers & by(B)).empty()) {
        if ((swap = int(score_t(B)) - swap) < res)
          break;
        occupied.reset(bb.front());
        attackers |= bishop(to, occupied) & by(B, Q);
      } else if (!(bb = stm_attackers & by(R)).empty()) {
        if ((swap = int(score_t(R)) - swap) < res)
          break;
        occupied.reset(bb.front());
        attackers |= rook(to, occupied) & by(R, Q);
      } else if (!(bb = stm_attackers & by(Q)).empty()) {
        if ((swap = int(score_t(Q)) - swap) < res)
          break;
        occupied.reset(bb.front());
        attackers |= (bishop(to, occupied) & by(B, Q)) | (rook(to, occupied) & by(R, Q));
      } else { // king: only legal if the opponent has no more attackers
        return (attackers & by(!stm)).empty() ? bool(res) : bool(res ^ 1);
      }
    }

    return bool(res);
  }

  bool is_no_material() const noexcept {
      return by(P).empty() && by(R).empty() && by(Q).empty() 
      && by(WHITE, N, B).size() <= 1 
      && by(BLACK, N, B).size()  <= 1;
  }

  bool is_50_moves_rule() const noexcept {
      return half_moves >= 100;
  }

  bool is_3_fold_repetition() const noexcept {
      const int s = int(history_size);
      const int limit = std::max(0, s - 2 * half_moves);
      size_t count = 0;
      for (int i = s - 2; i >= limit; i -= 2) {
          if (history[size_t(i)].hash == current_hash)
              count++;
      }
      return count >= 3;
  }

  std::span<move_t> generate_moves(const squares_t filter, const std::span<move_t, 256> buffer) const noexcept;
  void do_move(const move_t move) noexcept;
  void undo_move(const move_t move) noexcept;
  void do_null_move() noexcept;
  void undo_null_move() noexcept;

private:
  constexpr hash_t recompute_hash() const noexcept {
    hash_t result{};
    for (piece_t piece = WP; piece <= BK; ++piece)
      for (const square_t square : by(piece.side(), piece.type()))
        result ^= hash::lookup::piece(piece, square);
    for (const square_t square : castling_rights)
      result ^= hash::lookup::castle(square);
    if (!en_passant.empty())
      result ^= hash::lookup::ep(en_passant.front().file());
    if (side_to_move == BLACK)
      result ^= hash::lookup::side();
    return result;
  }

  std::array<piece_t, square_t::max> piece_at_square;
  std::array<squares_t, side_t::max> occupied_by_side;
  std::array<squares_t, type_t::max> occupied_by_type;
  squares_t en_passant;
  squares_t castling_rights;
  side_t side_to_move;
  uint8_t half_moves{};
  hash_t current_hash{};
  std::array<state_t, MAX_PLY> history{};
  size_t history_size = 0;
};

static_assert(sizeof(position_t) == 6320);
static_assert(position_t{}.by() == squares_t{_1, _2, _7, _8});
static_assert(position_t{}.by(WHITE) == squares_t{_1, _2});
static_assert(position_t{}.by(BLACK) == squares_t{_7, _8});
static_assert(position_t{}.by(P) == squares_t{_2, _7});
static_assert(position_t{}.by(N) == squares_t{b1, g1, b8, g8});
static_assert(position_t{}.by(B) == squares_t{c1, f1, c8, f8});
static_assert(position_t{}.by(R) == squares_t{a1, h1, a8, h8});
static_assert(position_t{}.by(Q) == squares_t{d1, d8});
static_assert(position_t{}.by(K) == squares_t{e1, e8});
static_assert(position_t{}.by(WHITE, P) == squares_t{_2});
static_assert(position_t{}.by(BLACK, P) == squares_t{_7});
static_assert(position_t{}.by(WHITE, N) == squares_t{b1, g1});
static_assert(position_t{}.by(BLACK, N) == squares_t{b8, g8});
static_assert(position_t{}.by(WHITE, B) == squares_t{c1, f1});
static_assert(position_t{}.by(BLACK, B) == squares_t{c8, f8});
static_assert(position_t{}.by(WHITE, R) == squares_t{a1, h1});
static_assert(position_t{}.by(BLACK, R) == squares_t{a8, h8});
static_assert(position_t{}.by(WHITE, Q) == squares_t{d1});
static_assert(position_t{}.by(BLACK, Q) == squares_t{d8});
static_assert(position_t{}.by(WHITE, K) == squares_t{e1});
static_assert(position_t{}.by(BLACK, K) == squares_t{e8});
static_assert(position_t{}.by(WHITE, R, Q) == squares_t{a1, h1, d1});
static_assert(position_t{}.by(BLACK, R, Q) == squares_t{a8, h8, d8});
static_assert(position_t{}.side() == WHITE);
static_assert(position_t{}.hash() == position_t{}.hash());
static_assert(position_t{}.hash() != position_t{"7k/8/8/8/8/8/8/K6R w - - 0 1"}.hash());

} // namespace chess
