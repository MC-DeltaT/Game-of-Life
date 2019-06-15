#include "utility.hpp"

#include <cstddef>
#include <cstdlib>
#include <string>
#define NOMINMAX
#include <Windows.h>


void set_console_size(HANDLE console_handle, std::size_t rows, std::size_t cols)
{
	auto rows_str = std::to_string(rows + 1);
	auto cols_str = std::to_string(cols + 1);
	auto str = "mode con cols=" + cols_str + " lines=" + rows_str;
	std::system(str.c_str());
}
