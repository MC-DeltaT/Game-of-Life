#pragma once

#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>
#define NOMINMAX
#include <Windows.h>


class thread_sync {
public:
	thread_sync();
	thread_sync(thread_sync const& other);

	void sync();
	template<typename Function>
	void sync(Function on_sync);
	void wait_for(std::size_t count);

private:
	struct sync_data {
		std::atomic_size_t count1;
		std::atomic_size_t count2;
	};

	std::shared_ptr<sync_data> _data;

	void _single_sync(std::atomic_size_t& count);
};


__forceinline void debug_assert(bool b)
{
#ifdef _DEBUG
	assert(b);
#endif
}

std::pair<std::size_t, std::size_t> get_console_size(HANDLE console_handle);

template<std::size_t NumPartitions>
constexpr std::array<std::pair<std::size_t, std::size_t>, NumPartitions> partition(std::size_t val);


#include "utility.tpp"
