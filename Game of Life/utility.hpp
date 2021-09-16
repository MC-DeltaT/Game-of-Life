#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>
#define NOMINMAX
#include <Windows.h>


template<typename T, std::size_t Alignment>
class aligned_array {
public:
	aligned_array(std::size_t size);

	T* data();
	T const* data() const;
	std::size_t size() const;

	T& operator[](std::size_t idx);
	T const& operator[](std::size_t idx) const;

	friend void swap(aligned_array& first, aligned_array& second)
	{
		std::swap(first._base, second._base);
		std::swap(first._data, second._data);
		std::swap(first._size, second._size);
	}

private:
	std::unique_ptr<T[]> _base;
	T* _data;
	std::size_t _size;
};


template<typename T>
T* align(T* buf, std::size_t actual_size, std::size_t required_size, std::size_t alignment);

__forceinline void debug_assert(bool b)
{
#ifdef _DEBUG
	assert(b);
#endif
}

template<std::size_t NumPartitions>
constexpr std::array<std::pair<std::size_t, std::size_t>, NumPartitions> partition(std::size_t val);

void set_console_size(HANDLE console_handle, std::size_t rows, std::size_t cols);

std::size_t wrap(std::ptrdiff_t x, std::size_t bound);


#include "utility.tpp"
