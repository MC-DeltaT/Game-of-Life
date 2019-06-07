#include "utility.hpp"

#include <cstddef>
#include <utility>
#include <vector>


void thread_barrier::wait()
{
	++_count1;
	while (_count1 > 0) {}
	++_count2;
	while (_count2 > 0) {}
}

void thread_barrier::wait_for(std::size_t count)
{
	while (_count1 < count) {}
	_count1 = 0;
	while (_count2 < count) {}
}

void thread_barrier::release()
{
	_count2 = 0;
}

void thread_barrier::sync_with(std::size_t count)
{
	wait_for(count);
	release();
}


std::vector<std::pair<std::size_t, std::size_t>> partition(std::size_t val, std::size_t partitions)
{
	std::vector<std::pair<std::size_t, std::size_t>> res;
	if (partitions > 0) {
		std::size_t const actual_partitions = val < partitions ? val : partitions;
		std::size_t const div = val / actual_partitions;
		std::size_t mod = val % actual_partitions;

		res.reserve(actual_partitions);
		std::size_t offset = 0;
		for (std::size_t i = 0; i < actual_partitions; ++i) {
			std::size_t extra = mod > 0 ? 1 : 0;
			res.emplace_back(offset, div + extra);
			offset += div + extra;
			mod -= extra;
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
