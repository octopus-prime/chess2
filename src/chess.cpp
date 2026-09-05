#include "position.hpp"
#include "attacks/generator.hpp"
#include <chrono>
#include <functional>
#include <print>
#include <algorithm>

namespace chess {

// struct perft_t {

//     constexpr perft_t(position_t &position) noexcept : position{position} {}

//     size_t operator()(const int depth) noexcept {
//         if (depth == 0)
//             return 1;

//         moves_t buffer{};
//         std::span<move_t> moves = position.generate_moves(ALL_SQUARES, buffer);
//         size_t count = 0;

//         const side_t side = position.side();
//         const square_t king = position.by(side, K).front();
//         const bool check = !position.checkers(side).empty();
//         const squares_t pinned = position.pinned(side);
//         const squares_t ep = position.ep();

//         for (const move_t move : moves) {
//             const bool is_ep = !ep.empty() && squares_t{move.to()} == ep && position.at(move.from()).type() == P;
//             const bool maybe_illegal = check || move.from() == king || (pinned & squares_t{move.from()}) || is_ep;
//             position.do_move(move);
//             if (!maybe_illegal || position.checkers(side).empty())
//                 count += (*this)(depth - 1);
//             position.undo_move(move);
//         }

//         return count;
//     }

// private:
//     position_t& position;
// };

constexpr int16_t evaluate(const position_t &position) noexcept {
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

    struct move_guard_t {
        move_guard_t(position_t &position, const move_t move) noexcept : position{position}, move{move} {
            position.do_move(move);
        }
        ~move_guard_t() noexcept {
            position.undo_move(move);
        }
    private:
        position_t &position;
        const move_t move;
    };

    constexpr std::pair<int16_t, int16_t> eval_move(const move_t &move) const noexcept {
        static constexpr std::array <int16_t, type_t::max> score = {
            100, 300, 300, 500, 1000, 20000, 0
        };
        int16_t piece = score[position.at(move.from()).type()];
        int16_t captured = score[position.at(move.to()).type()];
        return std::make_pair(captured, -piece);
    }

    constexpr void sort_moves(const std::span<move_t> moves) const noexcept {
        std::array<std::pair<int16_t, int16_t>, 256> move_scores{};
        auto moves_zip = std::views::zip(moves, move_scores);
        for (auto [move, score] : moves_zip)
            score = eval_move(move);
        std::ranges::sort(moves_zip, std::greater<>{}, [](const auto &tuple) { return std::get<1>(tuple); });
    }

    int qsearch(const int height, const int alpha, const int beta) noexcept {
        ++nodes;
        int stand_pat = evaluate(position);
        int best_score = alpha;

        if (stand_pat >= beta)
            return beta;
        if (stand_pat > best_score)
            best_score = stand_pat;

        moves_t buffer{};
        std::span<move_t> moves = position.generate_moves(position.by(~position.side()), buffer);
        sort_moves(moves);
        // std::array<std::pair<int16_t, int16_t>, 256> move_scores{};
        // auto moves_zip = std::views::zip(moves, move_scores);
        // for (auto [move, score] : moves_zip)
        //     score = eval_move(move);
        // std::ranges::sort(moves_zip, std::greater<>{}, [](const auto &tuple) { return std::get<1>(tuple); });

        const side_t side = position.side();
        const square_t king = position.by(side, K).front();
        const bool check = !position.checkers(side).empty();
        const squares_t pinned = position.pinned(side);
        const squares_t ep = position.ep();

        for (const move_t move : moves) {
            const move_guard_t guard{position, move};

            const bool is_ep = !ep.empty() && squares_t{move.to()} == ep && position.at(move.from()).type() == P;
            const bool maybe_illegal = check || move.from() == king || (pinned & squares_t{move.from()}) || is_ep;
            if (maybe_illegal && !position.checkers(side).empty())
                continue;

            int score = -qsearch(height + 1, -beta, -best_score);

            if (score >= beta)
                return beta;
            if (score > best_score)
                best_score = score;
        }

        return best_score;
    }

    int search(const int depth, const int height, const int alpha, const int beta) noexcept {
        if (depth == 0)
            return qsearch(height, alpha, beta);
        ++nodes;

        moves_t buffer{};
        std::span<move_t> moves = position.generate_moves(ALL_SQUARES, buffer);

        // if (moves.empty()) {
        //     const side_t side = position.side();
        //     const bool check = !position.checkers(side).empty();
        //     return check ? (-30000 + height) : 0;
        // }

        // std::array<std::pair<int16_t, int16_t>, 256> move_scores{};
        // auto moves_zip = std::views::zip(moves, move_scores);
        // for (auto [move, score] : moves_zip)
        //     score = eval_move(move);
        // std::ranges::sort(moves_zip, std::greater<>{}, [](const auto &tuple) { return std::get<1>(tuple); });

        sort_moves(moves);

        int score = alpha;
        size_t legal_moves = 0;

        const side_t side = position.side();
        const square_t king = position.by(side, K).front();
        const bool check = !position.checkers(side).empty();
        const squares_t pinned = position.pinned(side);
        const squares_t ep = position.ep();

        for (const move_t move : moves) {
            const move_guard_t guard{position, move};

            const bool is_ep = !ep.empty() && squares_t{move.to()} == ep && position.at(move.from()).type() == P;
            const bool maybe_illegal = check || move.from() == king || (pinned & squares_t{move.from()}) || is_ep;
            if (maybe_illegal && !position.checkers(side).empty())
                continue;
            ++legal_moves;

            int result = -search(depth - 1, height + 1, -beta, -score);

            if (result >= beta)
                return beta;
            if (result > score)
                score = result;
        }

        if (legal_moves == 0)
            return check ? (-30000 + height) : 0;

        return score;
    }

    int operator()(const int depth) noexcept {
        return search(depth,  0, -30000, 30000);
    }

    constexpr size_t nodes_searched() const noexcept { return nodes; }

private:
    position_t& position;
    size_t nodes;
};

// struct pvs_t {


//     constexpr pvs_t(position_t &position) noexcept : position{position}, nodes{0} {}

//     constexpr std::pair<int16_t, int16_t> eval_move(const move_t &move) const noexcept {
//     static constexpr std::array <int16_t, type_t::max> score = {
//         100, 300, 300, 500, 1000, 20000, 0
//     };
//         int16_t piece = score[position.at(move.from()).type()];
//         int16_t captured = score[position.at(move.to()).type()];
//         return std::make_pair(captured, -piece);
//     }

//     int qsearch(int alpha, int beta) noexcept {
//         ++nodes;

//         int stand_pat = evaluate(position);

//         if (stand_pat >= beta)
//             return beta;
//         if (stand_pat > alpha)
//             alpha = stand_pat;

//         moves_t buffer{};
//         std::span<move_t> moves = position.generate_moves(buffer);
//         auto tail = std::ranges::partition(moves, [&](const move_t &move) {
//             return position.at(move.to()) != NO_PIECE || move.promotion() != NO_TYPE;
//         });
//         auto result = std::ranges::subrange(moves.begin(), tail.begin());
//         std::array<std::pair<int16_t, int16_t>, 256> move_scores{};
//         auto moves_zip = std::views::zip(result, move_scores);
//         for (auto [move, score] : moves_zip)
//             score = eval_move(move);
//         std::ranges::sort(moves_zip, std::greater<>{}, [](const auto &tuple) { return std::get<1>(tuple); });

//         const side_t side = position.side();
//         const square_t king = position.by(side, K).front();
//         const bool check = !position.checkers(side).empty();
//         const squares_t pinned = position.pinned(side);
//         const squares_t ep = position.ep();

//         for (const move_t move : result) {
//             const bool is_ep = !ep.empty() && squares_t{move.to()} == ep && position.at(move.from()).type() == P;
//             const bool maybe_illegal = check || move.from() == king || (pinned & squares_t{move.from()}) || is_ep;
//             int score = 0;
//             bool valid = false;
//             position.do_move(move);
//             if (!maybe_illegal || position.checkers(side).empty()) {
//                 score = -qsearch(-beta, -alpha);
//                 valid = true;
//             }
//             position.undo_move(move);
//             if (valid && score >= beta) {
//                 return beta;
//             }
//             if (valid && score > alpha)
//                 alpha = score;
//         }

//         return alpha;
//     }

//     int operator()(const int depth, const int alpha, const int beta) noexcept {
//         const side_t side = position.side();

//         ++nodes;
//         if (depth == 0)
//             return evaluate(position);
//             // return qsearch(alpha, beta);

//         moves_t buffer{};
//         std::span<move_t> moves = position.generate_moves(buffer);
//         std::array<std::pair<int16_t, int16_t>, 256> move_scores{};
//         auto moves_zip = std::views::zip(moves, move_scores);
//         for (auto [move, score] : moves_zip)
//             score = eval_move(move);
//         std::ranges::sort(moves_zip, std::greater<>{}, [](const auto &tuple) { return std::get<1>(tuple); });
//         int score = alpha;

//         const square_t king = position.by(side, K).front();
//         const bool check = !position.checkers(side).empty();
//         const squares_t pinned = position.pinned(side);
//         const squares_t ep = position.ep();

//         for (const move_t move : moves) {
//             const bool is_ep = !ep.empty() && squares_t{move.to()} == ep && position.at(move.from()).type() == P;
//             const bool maybe_illegal = check || move.from() == king || (pinned & squares_t{move.from()}) || is_ep;
//             position.do_move(move);
//             if (!maybe_illegal || position.checkers(side).empty()) {
//                 if (score > alpha) {
//                     score = std::max<int>(score, -(*this)(depth - 1, -beta, -score));
//                 } else {
//                     score = std::max<int>(score, -(*this)(depth - 1, -alpha - 1, -score));
//                     if (score > alpha && score < beta)
//                         score = std::max<int>(score, -(*this)(depth - 1, -beta, -alpha));
//                 }
//                 if (score >= beta) {
//                     position.undo_move(move);
//                     return score;
//                 }
//             }
//             position.undo_move(move);
//         }
//         return score;
//     }

//     position_t &position;
//     size_t nodes;
// };

// void test_perft() {
//     position_t position{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
//     // position_t position{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"};
//     // position_t position{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"};
//     perft_t perft{position};
//     for (int i = 0; i < 8; ++i) {
//         const auto t0 = std::chrono::high_resolution_clock::now();
//         const size_t perft_result = perft(i);
//         const auto t1 = std::chrono::high_resolution_clock::now();
//         const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
//         // std::println("Speed (Mnodes/s): {:6.2f}", float(perft_result) / float(dt));
//         std::println("{:1d} {:12d} {:10} {:6.2f}MN/s", i, perft_result, dt, float(perft_result) / float(dt.count()) / 1000.f);
//     }
// }

void test_negamax() {
    position_t position{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
    // position_t position{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"};
    // position_t position{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"};
    // position_t position{"r1bqr3/ppp1B1kp/1b4p1/n2B4/3PQ1P1/2P5/P4P2/RN4K1 w - - 0 1"};
    // position_t position{"r1bq1rk1/ppp2ppp/2n2n2/3P4/2P1PB2/2N5/PP3PPP/R2Q1RK1 w - - 0 1"};
    negamax_t negamax{position};
        const auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 18; ++i) {
        const int score = negamax(i);
        const auto t1 = std::chrono::high_resolution_clock::now();
        const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
        // std::println("Speed (Mnodes/s): {:6.2f}", float(perft_result) / float(dt));
        std::println("{:>2d} {:>6d} {:>12d} {:>10} {:6.2f}MN/s", i, score, negamax.nodes_searched(), dt, float(negamax.nodes_searched()) / float(dt.count()) / 1000.f);
    }
}

// void test_pvs() {
//     // position_t position{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
//     // position_t position{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"};
//     // position_t position{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"};
//     position_t position{"2r3k1/pppR1pp1/4p3/4P1P1/5P2/1P4K1/P1P5/8 w - -"};
//     pvs_t pvs{position};
//     for (int i = 0; i < 18; ++i) {
//         const auto t0 = std::chrono::high_resolution_clock::now();
//         const int score = pvs(i, -30000, 30000);
//         const auto t1 = std::chrono::high_resolution_clock::now();
//         const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
//         // std::println("Speed (Mnodes/s): {:6.2f}", float(perft_result) / float(dt));
//         std::println("{:1d} {:<6} {:12d} {:10} {:6.2f}MN/s", i, score, pvs.nodes, dt, float(pvs.nodes) / float(dt.count()) / 1000.f);
//     }
// }

} // namespace chess

int main() {
    chess::test_negamax();
    // chess::test_pvs();
    return 0;
}

// int foo() {
//     using namespace chess;

//     constexpr position_t position{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"};
//     moves_t buffer{};
//     std::span<move_t> moves;
//     size_t count = 0;

//     for (int i = 0; i < 1000; ++i) {
//         position_t{}.generate_moves(buffer);
//         position_t{1}.generate_moves(buffer);
//         position.generate_moves(buffer);
//     }
//     const auto t0 = std::chrono::high_resolution_clock::now();
//     for (int i = 0; i < 1000000; ++i) {
//         moves = position_t{}.generate_moves(buffer);
//         count += moves.size();
//         moves = position_t{1}.generate_moves(buffer);
//         count += moves.size();
//         moves = position.generate_moves(buffer);
//         count += moves.size();
//     }
//     const auto t1 = std::chrono::high_resolution_clock::now();
//     const auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

//     std::println("Number of moves: {}", moves.size());
//     std::println("Total number of moves: {}", count);
//     std::println("Time taken (ns): {}", dt);
//     std::println("Speed (moves/s): {}", long(double(count) * 1'000'000'000.0 / double(dt)));
//     std::println("Generated moves = {}", moves);
//     std::println("foo = {}", move_t{e7, e8, Q});

//     return 0;    
// }
