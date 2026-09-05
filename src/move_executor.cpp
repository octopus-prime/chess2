#include "position.hpp"

namespace chess {

void position_t::do_move(const move_t move) noexcept {
    const square_t from = move.from();
    const square_t to = move.to();
    const piece_t piece = at(from);
    const squares_t prev_en_passant = en_passant;
    const squares_t prev_castling = castling_rights;

    square_t captured_square = to;
    piece_t captured = at(to);

    // pawn capturing diagonally onto the empty en-passant square takes the pawn beside it
    if (piece.type() == P && captured == NO_PIECE && squares_t{to} == prev_en_passant) {
        captured_square = square_e(int(to) + (piece.side() == WHITE ? -8 : 8));
        captured = at(captured_square);
    }

    const type_t promotion = move.promotion();
    const piece_t placed = promotion != NO_TYPE ? piece_t{piece.side(), promotion} : piece;

    piece_at_square[from] = piece_t{};
    piece_at_square[to] = placed;
    if (captured_square != to)
        piece_at_square[captured_square] = piece_t{};

    occupied_by_side[piece.side()].flip(squares_t{from, to});
    occupied_by_type[piece.type()].flip(squares_t{from});
    occupied_by_type[placed.type()].flip(squares_t{to});

    if (captured != NO_PIECE) {
        occupied_by_side[captured.side()].flip(squares_t{captured_square});
        occupied_by_type[captured.type()].flip(squares_t{captured_square});
    }

    // castling: a king move of two squares also relocates the corresponding rook
    if (piece.type() == K) {
        const int diff = int(to) - int(from);
        if (diff == 2 || diff == -2) {
            const square_t rook_from = square_e(diff == 2 ? int(from) + 3 : int(from) - 4);
            const square_t rook_to   = square_e(diff == 2 ? int(from) + 1 : int(from) - 1);
            piece_at_square[rook_to] = piece_at_square[rook_from];
            piece_at_square[rook_from] = piece_t{};
            occupied_by_side[piece.side()].flip(squares_t{rook_from, rook_to});
            occupied_by_type[R].flip(squares_t{rook_from, rook_to});
        }
    }

    en_passant = squares_t{};
    if (piece.type() == P) {
        const int diff = int(to) - int(from);
        if (diff == 16 || diff == -16)
            en_passant = squares_t{square_e((int(from) + int(to)) / 2)};
    }

    castling_rights &= ~squares_t{from, to};
    if (piece.type() == K)
        castling_rights &= ~(piece.side() == WHITE ? squares_t{a1, h1} : squares_t{a8, h8});

    side_to_move = !side_to_move;

    // history[history_size++] = {captured, captured_square, prev_en_passant, prev_castling};
    history[history_size++] = {captured, captured_square, prev_en_passant | prev_castling};
}

void position_t::undo_move(const move_t move) noexcept {
    const state_t state = history[--history_size];

    const square_t from = move.from();
    const square_t to = move.to();
    const piece_t placed = at(to);
    const type_t promotion = move.promotion();
    const piece_t piece = promotion != NO_TYPE ? piece_t{placed.side(), P} : placed;

    piece_at_square[to] = piece_t{};
    piece_at_square[from] = piece;
    if (state.captured != NO_PIECE)
        piece_at_square[state.captured_square] = state.captured;

    occupied_by_side[piece.side()].flip(squares_t{from, to});
    occupied_by_type[placed.type()].flip(squares_t{to});
    occupied_by_type[piece.type()].flip(squares_t{from});

    if (state.captured != NO_PIECE) {
        occupied_by_side[state.captured.side()].flip(squares_t{state.captured_square});
        occupied_by_type[state.captured.type()].flip(squares_t{state.captured_square});
    }

    // castling: move the rook back if this was a king move of two squares
    if (piece.type() == K) {
        const int diff = int(to) - int(from);
        if (diff == 2 || diff == -2) {
            const square_t rook_from = square_e(diff == 2 ? int(from) + 3 : int(from) - 4);
            const square_t rook_to   = square_e(diff == 2 ? int(from) + 1 : int(from) - 1);
            piece_at_square[rook_from] = piece_at_square[rook_to];
            piece_at_square[rook_to] = piece_t{};
            occupied_by_side[piece.side()].flip(squares_t{rook_from, rook_to});
            occupied_by_type[R].flip(squares_t{rook_from, rook_to});
        }
    }

    // en_passant = state.en_passant;
    // castling_rights = state.castling;
    en_passant = state.special & squares_t{_3, _6}; // mask for en_passant squares
    castling_rights = state.special & squares_t{_1, _8}; // mask for castling squares
    side_to_move = !side_to_move;
}

} // namespace chess
