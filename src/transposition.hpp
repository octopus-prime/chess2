#pragma once

#include "types/hash.hpp"
#include "types/score.hpp"
#include "move.hpp"
#include <cstdint>
#include <vector>
#include <optional>
#include <algorithm>

namespace chess {

enum flag_t : std::uint8_t {
	UNKNOWN,
	UPPER,
	LOWER,
	EXACT
};

class transposition_t {
	struct entry_t {
		using key_t = std::tuple<uint16_t, uint16_t>;
		key_t key;		//2*2
		move_t move;	//2
		score_t score;	//2
		flag_t flag;	//1
		depth_t	depth;	//1
	};

	static_assert(sizeof(entry_t) == 10);

	constexpr static size_t BUCKET_SIZE = 64 / sizeof(entry_t);

	static_assert(BUCKET_SIZE == 6);

	struct alignas(64) bucket_t {
		entry_t entries[BUCKET_SIZE]{};
	};

	static_assert(sizeof(bucket_t) == 64);

    std::vector<bucket_t> buckets;
	std::size_t used{0};

    constexpr static size_t BUCKETS_PER_MB = 1024 * 1024 / sizeof(bucket_t);

public:
    transposition_t(size_t mb = 16) : buckets(mb * BUCKETS_PER_MB + 137) {}
    // transposition_t() : buckets(1'000'037) {}
    // transposition_t() : buckets(499'999) {}
    // transposition_t() : buckets(250'007) {}
    // transposition_t() : buckets(125'003) {}

    void clear() noexcept {
        std::ranges::fill(buckets, bucket_t{});
		used = 0;
    }

    void put(const hash_t hash, const move_t move, const score_t score, const flag_t flag, const depth_t depth) noexcept {
		const entry_t::key_t key = {uint16_t(hash), uint16_t(hash >> 16)};
        bucket_t& bucket = buckets[hash % buckets.size()];
		entry_t* entry = std::ranges::find(bucket.entries, key, &entry_t::key);
		if (entry == std::end(bucket.entries)) {
			entry = std::ranges::min_element(bucket.entries, {}, &entry_t::depth);
			used += entry->flag == UNKNOWN;
		}
		*entry = {key, move, score, flag, depth};
    }

    std::optional<entry_t> get(const hash_t hash) const noexcept {
		const entry_t::key_t key = {uint16_t(hash), uint16_t(hash >> 16)};
        const bucket_t& bucket = buckets[hash % buckets.size()];
		const entry_t* entry = std::ranges::find(bucket.entries, key, &entry_t::key);
		if (entry == std::end(bucket.entries))
			return std::nullopt;
		return *entry;
    }

    size_t full() const noexcept {
		return size_t(1000.0 * double(used) / double(buckets.size() * BUCKET_SIZE));
    }
};

} // namespace chess
