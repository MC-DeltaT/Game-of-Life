#include "utility.hpp"

#include <cstddef>
#include <cstdlib>
#include <string>
#define NOMINMAX
#include <Windows.h>


void set_console_size(HANDLE console_handle, std::size_t rows, std::size_t cols)
{
	auto const rows_str = std::to_string(rows + 1);
	auto const cols_str = std::to_string(cols + 1);
	auto const str = "mode con cols=" + cols_str + " lines=" + rows_str;
	std::system(str.c_str());
}

std::size_t wrap(std::ptrdiff_t x, std::size_t bound)
{
	debug_assert(x >= -static_cast<std::ptrdiff_t>(bound));
	std::size_t res;
	if (x < 0) {
		res = x + bound;
	}
	else if (x >= bound) {
		res = x % bound;
	}
	else {
		res = x;
	}
	debug_assert(res < bound);
	return res;
}
