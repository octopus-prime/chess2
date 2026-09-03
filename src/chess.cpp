#include "position.hpp"
#include "generator.hpp"
#include "attacks/generator.hpp"
#include <chrono>
#include <functional>
#include <print>
#include <algorithm>

namespace chess {

struct perft_t {

    constexpr perft_t(position_t &position) noexcept : position{position} {}

    size_t operator()(const int depth) noexcept {
        if (depth == 0)
            return 1;

        generator::moves_t buffer{};
        std::span<move_t> moves = generator::generate_moves(position, buffer);
        size_t count = 0;

        const side_t side = position.side();
        const square_t king = position.by(side, K).front();
        const bool check = !position.checkers(side).empty();
        const squares_t pinned = position.pinned(side);

        for (const move_t move : moves) {
            const bool maybe_illegal = check || move.from() == king || (pinned & squares_t{move.from()});
            const piece_t captured = do_move(position, move);
            if (!maybe_illegal || position.checkers(side).empty())
                count += (*this)(depth - 1);
            undo_move(position, move, captured);
        }

        return count;
    }

private:
    position_t& position;
};

int16_t evaluate(const position_t &position) noexcept {
    const side_t side = position.side();
    int16_t score = 0;
    score += 20000 * (position.by(side, K).empty() - position.by(!side, K).empty());
    score += 1000 * (position.by(side, Q).size() - position.by(!side, Q).size());
    score += 500 * (position.by(side, R).size() - position.by(!side, R).size());
    score += 300 * (position.by(side, B).size() - position.by(!side, B).size());
    score += 300 * (position.by(side, N).size() - position.by(!side, N).size());
    score += 100 * (position.by(side, P).size() - position.by(!side, P).size());

    score += attacks::generator::leapers(position.by(side, K), position.by(side, N)).size();
    score -= attacks::generator::leapers(position.by(!side, K), position.by(!side, N)).size();

    score += attacks::generator::sliders(position.by(side, B, Q), position.by(side, R, Q), position.by()).size();
    score -= attacks::generator::sliders(position.by(!side, B, Q), position.by(!side, R, Q), position.by()).size();

    score += attacks::generator::pawns(position.by(side, P), side).size();
    score -= attacks::generator::pawns(position.by(!side, P), !side).size();

    return score;
}

struct negamax_t {

    constexpr negamax_t(position_t &position) noexcept : position{position}, nodes{0} {}

    int16_t operator()(const int depth, const int alpha, const int beta) noexcept {
        ++nodes;
        const side_t side = position.side();

        if (depth == 0)
            return evaluate(position);

        generator::moves_t buffer{};
        std::span<move_t> moves = generator::generate_moves(position, buffer);
        int16_t score = -30000;

        const square_t king = position.by(side, K).front();
        const bool check = !position.checkers(side).empty();
        const squares_t pinned = position.pinned(side);

        for (const move_t move : moves) {
            const bool maybe_illegal = check || move.from() == king || (pinned & squares_t{move.from()});
            const piece_t captured = do_move(position, move);
            if (!maybe_illegal || position.checkers(side).empty()) {
                score = std::max<int16_t>(score, int16_t(-(*this)(depth - 1, -beta, -alpha)));
                if (score >= beta) {
                    undo_move(position, move, captured);
                    return score;
                }
            }
            undo_move(position, move, captured);
        }

        return score;
    }

    constexpr size_t nodes_searched() const noexcept { return nodes; }

private:
    position_t& position;
    size_t nodes;
};

struct pvs_t {

    constexpr pvs_t(position_t &position) noexcept : position{position}, nodes{0} {}

    constexpr std::pair<int16_t, int16_t> eval_move(const move_t &move) const noexcept {
        static constexpr std::array <int16_t, type_t::max> score = {
            100, 300, 300, 500, 1000, 20000, 0
        };
        int16_t piece = score[position.at(move.from()).type()];
        int16_t captured = score[position.at(move.to()).type()];
        return std::make_pair(captured, -piece);
    }

    int16_t operator()(const int depth, const int alpha, const int beta) noexcept {
        ++nodes;
        const side_t side = position.side();

        if (depth == 0)
            return evaluate(position);

        generator::moves_t buffer{};
        std::span<move_t> moves = generator::generate_moves(position, buffer);
        std::array<std::pair<int16_t, int16_t>, 256> move_scores{};
        auto moves_zip = std::views::zip(moves, move_scores);
        for (auto [move, score] : moves_zip)
            score = eval_move(move);
        std::ranges::sort(moves_zip, std::greater<>{}, [](const auto &tuple) { return std::get<1>(tuple); });
        int16_t score = -30000;

        const square_t king = position.by(side, K).front();
        const bool check = !position.checkers(side).empty();
        const squares_t pinned = position.pinned(side);

        for (const move_t move : moves) {
            const bool maybe_illegal = check || move.from() == king || (pinned & squares_t{move.from()});
            const piece_t captured = do_move(position, move);
            if (!maybe_illegal || position.checkers(side).empty()) {
                if (score > alpha) {
                    score = std::max<int16_t>(score, int16_t(-(*this)(depth - 1, -beta, -alpha)));
                } else {
                    score = std::max<int16_t>(score, int16_t(-(*this)(depth - 1, -alpha - 1, -alpha)));
                    if (score > alpha && score < beta)
                        score = std::max<int16_t>(score, int16_t(-(*this)(depth - 1, -beta, -alpha)));
                }
                if (score >= beta) {
                    undo_move(position, move, captured);
                    return score;
                }
            }
            undo_move(position, move, captured);
        }
        return score;
    }

    position_t &position;
    size_t nodes;
};

void test_perft() {
    position_t position{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
    // position_t position{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"};
    // position_t position{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"};
    perft_t perft{position};
    for (int i = 0; i < 8; ++i) {
        const auto t0 = std::chrono::high_resolution_clock::now();
        const size_t perft_result = perft(i);
        const auto t1 = std::chrono::high_resolution_clock::now();
        const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
        // std::println("Speed (Mnodes/s): {:6.2f}", float(perft_result) / float(dt));
        std::println("{:1d} {:12d} {:10} {:6.2f}MN/s", i, perft_result, dt, float(perft_result) / float(dt.count()) / 1000.f);
    }
}

void test_negamax() {
    position_t position{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
    // position_t position{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"};
    // position_t position{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"};
    negamax_t negamax{position};
    for (int i = 0; i < 8; ++i) {
        const auto t0 = std::chrono::high_resolution_clock::now();
        const int16_t score = negamax(i, -30000, 30000);
        const auto t1 = std::chrono::high_resolution_clock::now();
        const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
        // std::println("Speed (Mnodes/s): {:6.2f}", float(perft_result) / float(dt));
        std::println("{:1d} {} {:12d} {:10} {:6.2f}MN/s", i, score, negamax.nodes_searched(), dt, float(negamax.nodes_searched()) / float(dt.count()) / 1000.f);
    }
}

void test_pvs() {
    position_t position{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
    // position_t position{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"};
    // position_t position{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"};
    pvs_t pvs{position};
    for (int i = 0; i < 8; ++i) {
        const auto t0 = std::chrono::high_resolution_clock::now();
        const int16_t score = pvs(i, -30000, 30000);
        const auto t1 = std::chrono::high_resolution_clock::now();
        const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
        // std::println("Speed (Mnodes/s): {:6.2f}", float(perft_result) / float(dt));
        std::println("{:1d} {} {:12d} {:10} {:6.2f}MN/s", i, score, pvs.nodes, dt, float(pvs.nodes) / float(dt.count()) / 1000.f);
    }
}

} // namespace chess

int main() {
    chess::test_perft();
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
