#include "executer.hpp"

namespace chess {

piece_t do_move(position_t &position, const move_t move) noexcept {
    const square_t from = move.from();
    const square_t to = move.to();
    const piece_t piece = position.at(from);
    const piece_t captured = position.at(to);

    position.piece_at_square[from] = piece_t{};
    position.piece_at_square[to] = piece;

    position.occupied_by_side[piece.side()].flip(squares_t{from, to});
    position.occupied_by_type[piece.type()].flip(squares_t{from, to});

    if (captured != NO_PIECE) {
        position.occupied_by_side[captured.side()].flip(squares_t{to});
        position.occupied_by_type[captured.type()].flip(squares_t{to});
    }

    position.side_to_move = !position.side_to_move;

    return captured;
}

void undo_move(position_t &position, const move_t move, const piece_t captured) noexcept {
    const square_t from = move.from();
    const square_t to = move.to();
    const piece_t piece = position.at(to);

    position.piece_at_square[to] = captured;
    position.piece_at_square[from] = piece;

    position.occupied_by_side[piece.side()].flip(squares_t{from, to});
    position.occupied_by_type[piece.type()].flip(squares_t{from, to});

    if (captured != NO_PIECE) {
        position.occupied_by_side[captured.side()].flip(squares_t{to});
        position.occupied_by_type[captured.type()].flip(squares_t{to});
    }

    position.side_to_move = !position.side_to_move;
}

} // namespace chess
