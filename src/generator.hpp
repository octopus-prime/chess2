#pragma once

#include "position.hpp"
#include "move.hpp"
#include <array>
#include <span>

namespace chess::generator {

using moves_t = std::array<move_t, 256>;

std::span<move_t> generate_moves(const position_t &position, const std::span<move_t, 256> moves) noexcept;

} // namespace chess::generator
