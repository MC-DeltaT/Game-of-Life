#pragma once

#include <array>
#include <cstddef>
#include <utility>


template<std::size_t NumPartitions>
constexpr std::array<std::pair<std::size_t, std::size_t>, NumPartitions> partition(std::size_t val)
{
	std::array<std::pair<std::size_t, std::size_t>, NumPartitions> res{};
	std::size_t const div = val / NumPartitions;
	std::size_t mod = val % NumPartitions;

	std::size_t offset = 0;
	for (std::size_t i = 0; i < NumPartitions; ++i) {
		res[i].first = offset;
		if (offset < val) {
			std::size_t const extra = mod > 0;
			std::size_t const count = div + extra;
			res[i].second = count;
			offset += count;
			mod -= extra;
		}
		else {
			res[i].second = 0;
		}
	}

	return res;
}
