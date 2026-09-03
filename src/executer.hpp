#pragma once

#include "position.hpp"
#include "move.hpp"

namespace chess {

struct undo_t {
  piece_t captured;
  square_t captured_square;
  squares_t en_passant;
};

undo_t do_move(position_t &position, const move_t move) noexcept;
void undo_move(position_t &position, const move_t move, const undo_t undo) noexcept;

} // namespace chess
