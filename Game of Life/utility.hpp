#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <utility>
#define NOMINMAX
#include <Windows.h>


__forceinline void debug_assert(bool b)
{
#ifdef _DEBUG
	assert(b);
#endif
}

template<std::size_t NumPartitions>
constexpr std::array<std::pair<std::size_t, std::size_t>, NumPartitions> partition(std::size_t val);

void set_console_size(HANDLE console_handle, std::size_t rows, std::size_t cols);


#include "utility.tpp"
