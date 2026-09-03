#pragma once

#include "position.hpp"
#include "move.hpp"

namespace chess {

void do_move(position_t &position, const move_t move) noexcept;
void undo_move(position_t &position, const move_t move) noexcept;

} // namespace chess
