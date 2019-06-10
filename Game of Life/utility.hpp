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

std::pair<std::size_t, std::size_t> get_console_size(HANDLE console_handle);

template<std::size_t NumPartitions>
constexpr std::array<std::pair<std::size_t, std::size_t>, NumPartitions> partition(std::size_t val);


#include "utility.tpp"
