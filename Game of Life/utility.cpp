#include "utility.hpp"

#include <cstddef>
#include <utility>
#define NOMINMAX
#include <Windows.h>


std::pair<std::size_t, std::size_t> get_console_size(HANDLE console_handle)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	debug_assert(GetConsoleScreenBufferInfo(console_handle, &csbi));
	return {csbi.srWindow.Bottom - csbi.srWindow.Top,
			csbi.srWindow.Right - csbi.srWindow.Left};
}
