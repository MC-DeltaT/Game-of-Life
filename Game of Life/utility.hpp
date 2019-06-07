#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>


class thread_barrier {
public:
	void wait();
	void wait_for(std::size_t count);
	void release();
	void sync_with(std::size_t count);

private:
	std::atomic_size_t _count1 = 0;
	std::atomic_size_t _count2 = 0;
};


__forceinline void debug_assert(bool b)
{
#ifdef _DEBUG
	assert(b);
#endif
}

std::vector<std::pair<std::size_t, std::size_t>> partition(std::size_t val, std::size_t partitions);
