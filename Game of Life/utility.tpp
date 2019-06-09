#pragma once

#include "utility.hpp"

#include <array>
#include <cstddef>
#include <utility>


template<typename Function>
inline void thread_sync::sync(Function on_sync)
{
	_single_sync(_data->count1);
	on_sync();
	_single_sync(_data->count2);
}


template<std::size_t NumPartitions>
constexpr std::array<std::pair<std::size_t, std::size_t>, NumPartitions> partition(std::size_t val)
{
	std::array<std::pair<std::size_t, std::size_t>, NumPartitions> res;
	std::size_t const div = val / NumPartitions;
	std::size_t mod = val % NumPartitions;

	std::size_t offset = 0;
	for (std::size_t i = 0; i < NumPartitions; ++i) {
		res[i].first = offset;
		if (offset < val) {
			std::size_t extra = mod > 0;
			std::size_t count = div + extra;
			res[i].second = count;
			offset += count;
			mod -= extra;
		}
		else {
			res[i].second = 0;
		}
	}

#ifdef _DEBUG
	std::size_t total = 0;
	std::size_t next_offset = 0;
	for (auto const& p : res) {
		debug_assert(p.first == next_offset);
		total += p.second;
		next_offset = p.first + p.second;
	}
	debug_assert(total == val);
#endif

	return res;
}
