#include "position.hpp"

namespace chess {

void position_t::do_move(const move_t move) noexcept {
    const square_t from = move.from();
    const square_t to = move.to();
    const piece_t piece = at(from);
    const squares_t prev_en_passant = en_passant;

    square_t captured_square = to;
    piece_t captured = at(to);

    // pawn capturing diagonally onto the empty en-passant square takes the pawn beside it
    if (piece.type() == P && captured == NO_PIECE && squares_t{to} == prev_en_passant) {
        captured_square = square_e(int(to) + (piece.side() == WHITE ? -8 : 8));
        captured = at(captured_square);
    }

    piece_at_square[from] = piece_t{};
    piece_at_square[to] = piece;
    if (captured_square != to)
        piece_at_square[captured_square] = piece_t{};

    occupied_by_side[piece.side()].flip(squares_t{from, to});
    occupied_by_type[piece.type()].flip(squares_t{from, to});

    if (captured != NO_PIECE) {
        occupied_by_side[captured.side()].flip(squares_t{captured_square});
        occupied_by_type[captured.type()].flip(squares_t{captured_square});
    }

    en_passant = squares_t{};
    if (piece.type() == P) {
        const int diff = int(to) - int(from);
        if (diff == 16 || diff == -16)
            en_passant = squares_t{square_e((int(from) + int(to)) / 2)};
    }

    side_to_move = !side_to_move;

    history[history_size++] = {captured, captured_square, prev_en_passant};
}

void position_t::undo_move(const move_t move) noexcept {
    const state_t state = history[--history_size];

    const square_t from = move.from();
    const square_t to = move.to();
    const piece_t piece = at(to);

    piece_at_square[to] = piece_t{};
    piece_at_square[from] = piece;
    if (state.captured != NO_PIECE)
        piece_at_square[state.captured_square] = state.captured;

    occupied_by_side[piece.side()].flip(squares_t{from, to});
    occupied_by_type[piece.type()].flip(squares_t{from, to});

    if (state.captured != NO_PIECE) {
        occupied_by_side[state.captured.side()].flip(squares_t{state.captured_square});
        occupied_by_type[state.captured.type()].flip(squares_t{state.captured_square});
    }

    en_passant = state.en_passant;
    side_to_move = !side_to_move;
}

} // namespace chess
