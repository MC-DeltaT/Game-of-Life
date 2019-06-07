#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>


class thread_sync {
public:
	thread_sync();
	thread_sync(thread_sync const& other);

	void sync();
	template<typename Callable>
	void sync(Callable on_sync);
	std::size_t waiting();

private:
	struct sync_data {
		std::atomic_size_t count1;
		std::atomic_size_t count2;
	};

	std::shared_ptr<sync_data> _data;

	void _single_sync(std::atomic_size_t& count);
};

template<typename Callable>
inline void thread_sync::sync(Callable on_sync)
{
	_single_sync(_data->count1);
	on_sync();
	_single_sync(_data->count2);
}


__forceinline void debug_assert(bool b)
{
#ifdef _DEBUG
	assert(b);
#endif
}

std::vector<std::pair<std::size_t, std::size_t>> partition(std::size_t val, std::size_t partitions);
