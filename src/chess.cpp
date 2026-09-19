#include "position.hpp"
#include "searcher.hpp"

void test_search() {
    using namespace chess;
    // position_t position{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"}; // startpos
    // position_t position{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"}; // kiwepete
    // position_t position{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"}; // matt in 2
    // position_t position{"r1bqr3/ppp1B1kp/1b4p1/n2B4/3PQ1P1/2P5/P4P2/RN4K1 w - - 0 1"}; // matt in 4
    // position.do_move(move_t{e4, e5});
    // position_t position{"r1bq1rk1/ppp2ppp/2n2n2/3P4/2P1PB2/2N5/PP3PPP/R2Q1RK1 b - - 0 1"};

    // position_t position{"4k3/8/8/8/8/8/4P3/4K1N1 w - -"};
    // position_t position{"4k3/4p3/8/8/8/8/3PP3/4K1N1 w - - 0 1"};
    position_t position{"4k3/3pp3/8/8/8/8/3PP3/4K1N1 w - - 0 1"};
    // position_t position{"8/4k3/3pp3/8/3PP3/4KN2/8/8 w - - 0 1"};
    // position_t position{"8/4k3/3pp3/8/8/4KN2/8/8 w - - 0 1"};
    // position_t position{"8/4k3/8/8/2p5/4KN2/8/8 w - - 0 1"};

    // position_t position{"1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - -"};
    // position_t position{"3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - -"};
    // position_t position{"2q1rr1k/3bbnnp/p2p1pp1/2pPp3/PpP1P1P1/1P2BNNP/2BQ1PRK/7R b - -"};
    // position_t position{"rnbqkb1r/p3pppp/1p6/2ppP3/3N4/2P5/PPP1QPPP/R1B1KB1R w KQkq -"};
    // position_t position{"r1b2rk1/2q1b1pp/p2ppn2/1p6/3QP3/1BN1B3/PPP3PP/R4RK1 w - -"};
    // position_t position{"2r3k1/pppR1pp1/4p3/4P1P1/5P2/1P4K1/P1P5/8 w - -"};
    // position_t position{"1nk1r1r1/pp2n1pp/4p3/q2pPp1N/b1pP1P2/B1P2R2/2P1B1PP/R2Q2K1 w - -"};
    // position_t position{"4b3/p3kp2/6p1/3pP2p/2pP1P2/4K1P1/P3N2P/8 w - -"};
    // position_t position{"2kr1bnr/pbpq4/2n1pp2/3p3p/3P1P1B/2N2N1Q/PPP3PP/2KR1B1R w - -"};
    // position_t position{"3rr1k1/pp3pp1/1qn2np1/8/3p4/PP1R1P2/2P1NQPP/R1B3K1 b - -"};

    negamax_t negamax{position};
    // negamax_t{"3rr1k1/pp3pp1/1qn2np1/8/3p4/PP1R1P2/2P1NQPP/R1B3K1 b - -"}(255, std::chrono::seconds(30));
    // negamax(18);
    negamax(255, std::chrono::seconds(30));
    // negamax(15, std::chrono::steady_clock::duration::max() / 2);
}


int main() {
    test_search();
    return 0;
}
