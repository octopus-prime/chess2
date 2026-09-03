#include "executer.hpp"

namespace chess {

undo_t do_move(position_t &position, const move_t move) noexcept {
    const square_t from = move.from();
    const square_t to = move.to();
    const piece_t piece = position.at(from);
    const squares_t prev_en_passant = position.ep();

    square_t captured_square = to;
    piece_t captured = position.at(to);

    // pawn capturing diagonally onto the empty en-passant square takes the pawn beside it
    if (piece.type() == P && captured == NO_PIECE && squares_t{to} == prev_en_passant) {
        captured_square = square_e(int(to) + (piece.side() == WHITE ? -8 : 8));
        captured = position.at(captured_square);
    }

    position.piece_at_square[from] = piece_t{};
    position.piece_at_square[to] = piece;
    if (captured_square != to)
        position.piece_at_square[captured_square] = piece_t{};

    position.occupied_by_side[piece.side()].flip(squares_t{from, to});
    position.occupied_by_type[piece.type()].flip(squares_t{from, to});

    if (captured != NO_PIECE) {
        position.occupied_by_side[captured.side()].flip(squares_t{captured_square});
        position.occupied_by_type[captured.type()].flip(squares_t{captured_square});
    }

    position.en_passant = squares_t{};
    if (piece.type() == P) {
        const int diff = int(to) - int(from);
        if (diff == 16 || diff == -16)
            position.en_passant = squares_t{square_e((int(from) + int(to)) / 2)};
    }

    position.side_to_move = !position.side_to_move;

    return {captured, captured_square, prev_en_passant};
}

void undo_move(position_t &position, const move_t move, const undo_t undo) noexcept {
    const square_t from = move.from();
    const square_t to = move.to();
    const piece_t piece = position.at(to);

    position.piece_at_square[to] = piece_t{};
    position.piece_at_square[from] = piece;
    if (undo.captured != NO_PIECE)
        position.piece_at_square[undo.captured_square] = undo.captured;

    position.occupied_by_side[piece.side()].flip(squares_t{from, to});
    position.occupied_by_type[piece.type()].flip(squares_t{from, to});

    if (undo.captured != NO_PIECE) {
        position.occupied_by_side[undo.captured.side()].flip(squares_t{undo.captured_square});
        position.occupied_by_type[undo.captured.type()].flip(squares_t{undo.captured_square});
    }

    position.en_passant = undo.en_passant;
    position.side_to_move = !position.side_to_move;
}

} // namespace chess
