#include "utility.hpp"

#include <cstddef>
#include <utility>
#include <vector>
#define NOMINMAX
#include <Windows.h>


thread_sync::thread_sync() :
	_data(new sync_data{})
{
	_data->count1 = 0;
	_data->count2 = 0;
}

thread_sync::thread_sync(thread_sync const& other)
{
	debug_assert(other._data->count2 == 0);
	_data = other._data;
}


void thread_sync::sync()
{
	_single_sync(_data->count1);
	_single_sync(_data->count2);
}

void thread_sync::wait_for(std::size_t count)
{
	while (_data->count1 < count) {}
}

void thread_sync::_single_sync(std::atomic_size_t& count)
{
	bool last_thread = ++count == _data.use_count();
	if (last_thread) {
		count = 0;
	}
	else {
		while (count > 0) {}
	}
}


std::pair<std::size_t, std::size_t> get_console_size(HANDLE console_handle)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	debug_assert(GetConsoleScreenBufferInfo(console_handle, &csbi));
	return {csbi.srWindow.Bottom - csbi.srWindow.Top,
			csbi.srWindow.Right - csbi.srWindow.Left};
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
