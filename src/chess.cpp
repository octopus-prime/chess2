#include "position.hpp"
#include "generator.hpp"
#include <chrono>
#include <print>

namespace chess {

size_t perft(position_t &position, const int depth) noexcept {
    if (depth == 0)
        return 1;

    generator::moves_t buffer{};
    std::span<move_t> moves = generator::generate_moves(position, buffer);
    size_t count = 0;

    for (const move_t move : moves) {
        const piece_t captured = do_move(position, move);
        if (!position.check(!position.side()))
            count += perft(position, depth - 1);
        undo_move(position, move, captured);
    }

    return count;
}

void test() {
    // position_t position{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
    // position_t position{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"};
    position_t position{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"};
    for (int i = 0; i < 8; ++i) {
        std::print("Perft depth {}: ", i);
        const auto t0 = std::chrono::high_resolution_clock::now();
        const size_t perft_result = perft(position, i);
        const auto t1 = std::chrono::high_resolution_clock::now();
        const auto dt = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        std::println("{}", perft_result);
        std::println("Time taken (µs): {}", dt);
        std::println("Speed (Mnodes/s): {:6.2f}", float(perft_result) / float(dt));
    }
}

} // namespace chess

int main() {
    chess::test();
    return 0;
}

int foo() {
    using namespace chess;

    constexpr position_t position{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"};
    generator::moves_t buffer{};
    std::span<move_t> moves;
    size_t count = 0;

    for (int i = 0; i < 1000; ++i) {
        generator::generate_moves(position_t{}, buffer);
        generator::generate_moves(position_t{1}, buffer);
        generator::generate_moves(position, buffer);
    }
    const auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; ++i) {
        moves = generator::generate_moves(position_t{}, buffer);
        count += moves.size();
        moves = generator::generate_moves(position_t{1}, buffer);
        count += moves.size();
        moves = generator::generate_moves(position, buffer);
        count += moves.size();
    }
    const auto t1 = std::chrono::high_resolution_clock::now();
    const auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

    std::println("Number of moves: {}", moves.size());
    std::println("Total number of moves: {}", count);
    std::println("Time taken (ns): {}", dt);
    std::println("Speed (moves/s): {}", long(double(count) * 1'000'000'000.0 / double(dt)));
    std::println("Generated moves = {}", moves);
    std::println("foo = {}", move_t{e7, e8, Q});

    return 0;    
}
