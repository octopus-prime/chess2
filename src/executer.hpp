#pragma once

#include "position.hpp"
#include "move.hpp"

namespace chess {

piece_t do_move(position_t &position, const move_t move) noexcept;
void undo_move(position_t &position, const move_t move, const piece_t captured) noexcept;

} // namespace chess
