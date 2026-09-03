#include "position.hpp"
#include "attacks/lookup.hpp"
#include "types/squares.hpp"

namespace chess {

constexpr __v64qu AllSquares = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};

inline size_t splat_leapers(const square_t from, const squares_t targets, const std::span<move_t> buffer) noexcept {
    // assert(count <= 8);  // max 8 attacks
    constexpr square_e placeholder{0};

    const __v8hu vec_from = _mm_set1_epi16(int16_t(move_t(from, placeholder)));
    const __v8hu vec_to = _mm_cvtepi8_epi16(_mm512_castsi512_si128(_mm512_maskz_compress_epi8(targets, AllSquares)));
    const __v8hu moves = vec_from | vec_to << move_t::TO_SQ_SHIFT;

    _mm_storeu_si128(reinterpret_cast<__m128i*>(buffer.data()), moves);
    return targets.size();
}

inline size_t splat_sliders(const square_t from, const squares_t targets, const std::span<move_t> buffer) noexcept {
    // assert(count <= 16);  // max 16 attacks
    constexpr square_e placeholder{0};

    const __v16hu vec_from = _mm256_set1_epi16(int16_t(move_t(from, placeholder)));
    const __v16hu vec_to = _mm256_cvtepi8_epi16(_mm512_castsi512_si128(_mm512_maskz_compress_epi8(targets, AllSquares)));
    const __v16hu moves = vec_from | vec_to << move_t::TO_SQ_SHIFT;

    _mm256_storeu_si256(reinterpret_cast<__m256i*>(buffer.data()), moves);
    return targets.size();
}

inline size_t splat_pawns(const int16_t offset, const squares_t targets, const std::span<move_t> buffer) noexcept {
    // assert(count <= 8);  // max 8 attacks
    constexpr __v8hu promotion = __v8hu{NO_TYPE, NO_TYPE, NO_TYPE, NO_TYPE, NO_TYPE, NO_TYPE, NO_TYPE, NO_TYPE} << move_t::PROMOTION_SHIFT;

    const __v8hu vec_to    = _mm_cvtepi8_epi16(_mm512_castsi512_si128(_mm512_maskz_compress_epi8(targets, AllSquares)));
    const __v8hu vec_from  = vec_to + uint16_t(offset);
    const __v8hu moves = vec_from << move_t::FROM_SQ_SHIFT | vec_to << move_t::TO_SQ_SHIFT | promotion;

    _mm_storeu_si128(reinterpret_cast<__m128i*>(buffer.data()), moves);
    return targets.size();
}

std::span<move_t> position_t::generate_moves(const std::span<move_t, 256> buffer) const noexcept {
    using namespace attacks::lookup;

    const side_t side = this->side();
    const squares_t occupied = by();
    const squares_t empty = ~by();
    const squares_t enemies = by(!side);
    const squares_t not_us = ~by(side);

    size_t count = 0;

    {
        const square_t from = by(side, K).front();
        count += splat_leapers(from, king(from) & not_us, buffer.subspan(count));
    }

    for (const square_t from : by(side, N))
        count += splat_leapers(from, knight(from) & not_us, buffer.subspan(count));

    for (const square_t from : by(side, R, Q))
        count += splat_sliders(from, rook(from, occupied) & not_us, buffer.subspan(count));

    for (const square_t from : by(side, B, Q))
        count += splat_sliders(from, bishop(from, occupied) & not_us, buffer.subspan(count));

    const squares_t pawns = by(side, P);
    const squares_t ep = this->ep();
    if (side == WHITE) {
        const squares_t push1 = (pawns << 8) & empty;
        const squares_t push2 = ((pawns & squares_t{_2}) << 8 & empty) << 8 & empty;
        const squares_t left  = (pawns << 7) & ~squares_t{h} & (enemies | ep);
        const squares_t right = (pawns << 9) & ~squares_t{a} & (enemies | ep);

        count += splat_pawns(-8, push1, buffer.subspan(count));
        count += splat_pawns(-16, push2, buffer.subspan(count));
        count += splat_pawns(-7, left, buffer.subspan(count));
        count += splat_pawns(-9, right, buffer.subspan(count));
    } else {
        const squares_t push1 = (pawns >> 8) & empty;
        const squares_t push2 = ((pawns & squares_t{_7}) >> 8 & empty) >> 8 & empty;
        const squares_t left  = (pawns >> 9) & ~squares_t{h} & (enemies | ep);
        const squares_t right = (pawns >> 7) & ~squares_t{a} & (enemies | ep);

        count += splat_pawns(+8, push1, buffer.subspan(count));
        count += splat_pawns(+16, push2, buffer.subspan(count));
        count += splat_pawns(+9, left, buffer.subspan(count));
        count += splat_pawns(+7, right, buffer.subspan(count));
    }

    return buffer.first(count);
}

} // namespace chess
