#pragma once

#include "types/side.hpp"
#include "types/square.hpp"
#include "types/piece.hpp"
#include "types/squares.hpp"
#include "attacks/lookup.hpp"
#include <array>
#include <ranges>

namespace chess {

struct move_t;

struct position_t final {
  constexpr position_t(int) noexcept
   : piece_at_square{}
   , occupied_by_side{
    squares_t{_1}, // WHITE
    squares_t{_8} // BLACK
    }
   , occupied_by_type{
    squares_t{}, // PAWN
    squares_t{b1, g1, b8, g8}, // KNIGHT
    squares_t{c1, f1, c8, f8}, // BISHOP
    squares_t{a1, h1, a8, h8}, // ROOK
    squares_t{d1, d8}, // QUEEN
    squares_t{e1, e8} // KING
   }
   , side_to_move{WHITE} {}

  constexpr position_t() noexcept
   : piece_at_square{}
   , occupied_by_side{
    squares_t{_1, _2}, // WHITE
    squares_t{_7, _8} // BLACK
    }
   , occupied_by_type{
    squares_t{_2, _7}, // PAWN
    squares_t{b1, g1, b8, g8}, // KNIGHT
    squares_t{c1, f1, c8, f8}, // BISHOP
    squares_t{a1, h1, a8, h8}, // ROOK
    squares_t{d1, d8}, // QUEEN
    squares_t{e1, e8} // KING
   }
   , side_to_move{WHITE} {}

  constexpr position_t(std::string_view fen) noexcept {
    // constexpr auto castle_lookup = [](char ch) static -> squares_t {
    //     switch (ch) {
    //         case 'K': return squares_t{h1}; case 'k': return squares_t{h8};
    //         case 'Q': return squares_t{a1}; case 'q': return squares_t{a8};
    //         default: return squares_t{};
    //     }
    // };
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
    // if (castle_part[0] != '-') {
    //     for (auto ch : castle_part) {
    //         new_state.castle |= castle_lookup(ch);
    //     }
    //     new_state.hash ^= hashes::castle(new_state.castle);
    // }

    std::string_view en_passant_part {*fen_part++};
    // if (en_passant_part[0] != '-') {
    //     new_state.en_passant = en_passant_part;
    //     new_state.hash ^= hashes::en_passant(new_state.en_passant);
    // } else {
    //     new_state.en_passant = NO_SQUARE;
    // }

    // if (fen_part != fen_parts.end()) {
    //     std::string_view half_move_part {*fen_part++};
    //     std::from_chars(&*half_move_part.begin(), &*half_move_part.end(), new_state.half_move);
    // }

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
  }

  constexpr piece_t at(const square_t square) const noexcept { return piece_at_square[square]; }
  constexpr squares_t by() const noexcept { return by(WHITE) | by(BLACK); }
  constexpr squares_t by(const side_t side) const noexcept { return occupied_by_side[side]; }
  constexpr squares_t by(const type_t type) const noexcept { return occupied_by_type[type]; }
  constexpr squares_t by(const piece_t piece) const noexcept { return by(piece.side()) & by(piece.type()); }
  constexpr squares_t by(const side_t side, const type_t type) const noexcept { return by(side) & by(type); }
  constexpr squares_t by(const type_t type1, const type_t type2) const noexcept { return by(type1) | by(type2); }
  constexpr squares_t by(const side_t side, const type_t type1, const type_t type2) const noexcept { return by(side) & (by(type1, type2)); }
  constexpr side_t side() const noexcept { return side_to_move; }

  constexpr bool check(side_t side) const noexcept {
    const square_t king = by(side, K).front();
    return attacks::lookup::king(king) & by(!side, K) ||
           attacks::lookup::knight(king) & by(!side, N) ||
           attacks::lookup::rook(king, by()) & by(!side, R, Q) ||
           attacks::lookup::bishop(king, by()) & by(!side, B, Q) ||
           attacks::lookup::pawn(king, side) & by(!side, P);
  }

  friend piece_t do_move(position_t &position, const move_t move) noexcept;
  friend void undo_move(position_t &position, const move_t move, const piece_t captured) noexcept;

private:
  std::array<piece_t, square_t::max> piece_at_square;
  std::array<squares_t, side_t::max> occupied_by_side;
  std::array<squares_t, type_t::max> occupied_by_type;
  side_t side_to_move;
};

// static_assert(sizeof(position_t) == 136);
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

} // namespace chess
