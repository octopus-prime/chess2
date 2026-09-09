#pragma once

#include "position.hpp"
#include "attacks/generator.hpp"
#include "transposition.hpp"
#include "types/square.hpp"
#include <chrono>
#include <functional>
// #include <print>
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

constexpr score_t evaluate(const position_t &position) noexcept {
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

    return score_e(score);
}


struct history_t {
    std::array<std::array<uint64_t, piece_t::max>, square_t::max> piece_to{};
    std::array<std::array<uint64_t, square_t::max>, square_t::max> from_to{};

    void put(const move_t move, const piece_t piece, const depth_t depth) noexcept {
        piece_to[move.to()][piece] += 1ull << depth;
        from_to[move.to()][move.from()] += 1ull << depth;
    }

    uint64_t get(const move_t move, const piece_t piece) const noexcept {
        return (piece_to[move.to()][piece] + from_to[move.to()][move.from()]) / 2;
    }
};

struct negamax_t {

    constexpr negamax_t(position_t &position) noexcept : position{position}, nodes{0} {}

    using scores4_t = std::tuple<bool, int16_t, uint32_t>;

    constexpr int16_t eval_move(const move_t move) const noexcept {
        static constexpr std::array <int16_t, type_t::max> score = {
            100, 300, 300, 500, 1000, 20000, 0
        };
        int16_t piece = score[position.at(move.from()).type()];
        int16_t captured = score[position.at(move.to()).type()];
        return captured - piece / 10;
    }

    constexpr void sort_moves(const std::span<move_t> moves) const noexcept {
        std::array<int16_t, 256> move_scores{};
        auto moves_zip = std::views::zip(moves, move_scores);
        for (auto [move, score] : moves_zip)
            score = eval_move(move);
        std::ranges::sort(moves_zip, std::greater<>{}, [](const auto &tuple) { return std::get<1>(tuple); });
    }

    constexpr scores4_t eval_move(const move_t move, const move_t best, const depth_t depth) const noexcept {
        static constexpr std::array <int16_t, type_t::max> score = {
            100, 300, 300, 500, 1000, 20000, 0
        };
        int16_t piece = score[position.at(move.from()).type()];
        int16_t captured = score[position.at(move.to()).type()];
        return std::make_tuple(move == best, captured - piece / 10, uint32_t(history.get(move, position.at(move.from())) ) >> (depth - 1));
    }

    constexpr void sort_moves(const std::span<move_t> moves, const move_t best, const depth_t depth) const noexcept {
        std::array<scores4_t, 256> move_scores{};
        auto moves_zip = std::views::zip(moves, move_scores);
        for (auto [move, score] : moves_zip)
            score = eval_move(move, best, depth);
        std::ranges::sort(moves_zip, std::greater<>{}, [](const auto &tuple) { return std::get<1>(tuple); });
    }

    score_t qsearch(const depth_t height, const score_t alpha, const score_t beta) noexcept {
        ++nodes;
        const score_t stand_pat = evaluate(position);
        score_t best_score = alpha;

        if (stand_pat >= beta)
            return beta;
        if (stand_pat > best_score)
            best_score = stand_pat;

        moves_t buffer{};
        std::span<move_t> moves = position.generate_moves(position.by(~position.side()), buffer);
        sort_moves(moves);

        const side_t side = position.side();
        const square_t king = position.by(side, K).front();
        const bool check = !position.checkers(side).empty();
        const squares_t pinned = position.pinned(side);
        const squares_t ep = position.ep();

        for (const move_t move : moves) {
            position.do_move(move);

            const bool is_ep = !ep.empty() && squares_t{move.to()} == ep && position.at(move.from()).type() == P;
            const bool maybe_illegal = check || move.from() == king || (pinned & squares_t{move.from()}) || is_ep;
            if (maybe_illegal && !position.checkers(side).empty()) {
                position.undo_move(move);
                continue;
            }

            const score_t score = -qsearch(height + 1, -beta, -best_score);

            position.undo_move(move);

            if (score >= beta)
                return beta;
            if (score > best_score)
                best_score = score;
        }

        return best_score;
    }

    score_t search(const depth_t depth, const depth_t height, score_t alpha, score_t beta) noexcept {
        if (depth == 0)
            return qsearch(height, alpha, beta);
        ++nodes;

        move_t best_move{};
        if (const auto entry = transposition.get(position.hash())) {
            best_move = entry->move;
            if (entry->depth >= depth) {
                switch (entry->flag) {
                case flag_t::EXACT:
                    return entry->score;
                case flag_t::LOWER:
                    if (entry->score > alpha)
                        alpha = entry->score;
                    break;
                case flag_t::UPPER:
                    if (entry->score < beta)
                        beta = entry->score;
                    break;
                default:
                    break;
                }
                if (alpha >= beta)
                    return beta;
            }
        }

        const side_t side = position.side();
        const bool check = !position.checkers(side).empty();

        const score_t blub = evaluate(position);
        if (!check && blub >= beta + score_e(100 + 200 * depth) && depth < 3 && !beta.is_decisive())
            return beta;

        moves_t buffer{};
        std::span<move_t> moves = position.generate_moves(ALL_SQUARES, buffer);

        // if (moves.empty())
        //     return check ? score_e(-30000 + height) : DRAW;

        sort_moves(moves, best_move, depth);

        score_t best_score = alpha;
        size_t legal_moves = 0;

        const square_t king = position.by(side, K).front();
        const squares_t pinned = position.pinned(side);
        const squares_t ep = position.ep();

        for (const move_t move : moves) {
            position.do_move(move);

            const bool is_ep = !ep.empty() && squares_t{move.to()} == ep && position.at(move.from()).type() == P;
            const bool maybe_illegal = check || move.from() == king || (pinned & squares_t{move.from()}) || is_ep;
            if (maybe_illegal && !position.checkers(side).empty()) {
                position.undo_move(move);
                continue;
            }
            ++legal_moves;

            const score_t score = -search(depth - 1, height + 1, -beta, -best_score);

            position.undo_move(move);

            if (score >= beta) {
                transposition.put(position.hash(), move, beta, flag_t::LOWER, depth);
                history.put(move, position.at(move.from()), depth);
                return beta;
            }
            if (score > best_score) {
                best_score = score;
                best_move = move;
            }
        }

        if (legal_moves == 0)
            return check ? score_e(MIN + height) : DRAW;

        if (best_score > alpha) {
            transposition.put(position.hash(), best_move, best_score, flag_t::EXACT, depth);
            history.put(best_move, position.at(best_move.from()), depth);
        } else {
            transposition.put(position.hash(), best_move, best_score, flag_t::UPPER, depth);
        }

        return best_score;
    }

    void operator()(const depth_t max_depth) noexcept {
        moves_t buffer{};
        char output[1024];
        const auto t0 = std::chrono::high_resolution_clock::now();
        for (depth_t depth = 1; depth < max_depth; ++depth) {
            const score_t score = search(depth,  0, MIN, MAX);
            const auto t1 = std::chrono::high_resolution_clock::now();
            const auto time = duration_cast<std::chrono::duration<double, std::ratio<1>>>(t1 - t0).count();

            const char* out = std::format_to(output, "info depth {} seldepth {} score {} nodes {} nps {} hashfull {} time {} pv {}\n",
                        depth, 0, score, nodes, size_t(double(nodes) / time), transposition.full(), size_t(time * 1000), extract_pv(buffer));
            std::fwrite(output, sizeof(char), size_t(out - output), stdout);
        }
        // return search(depth,  0, MIN, MAX);
    }

    // constexpr size_t nodes_searched() const noexcept { return nodes; }

    // constexpr size_t transposition_full() const noexcept { return transposition.full(); }

    // follows TT best-moves from the current position, replaying then unwinding them to leave position unchanged
    std::span<move_t> extract_pv(const std::span<move_t, 256> buffer) noexcept {
        size_t count = 0;
        while (count < buffer.size()) {
            const auto entry = transposition.get(position.hash());
            if (!entry || entry->move == move_t{})
                break;
            buffer[count++] = entry->move;
            position.do_move(entry->move);
        }
        for (size_t i = count; i-- > 0;)
            position.undo_move(buffer[i]);
        return buffer.first(count);
    }

private:
    position_t& position;
    size_t nodes{};
    transposition_t transposition{64};
    history_t history{};
};

} // namespace chess
