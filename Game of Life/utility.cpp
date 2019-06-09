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
