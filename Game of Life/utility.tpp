#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>


template<typename T, std::size_t Alignment>
aligned_array<T, Alignment>::aligned_array(std::size_t size) :
	_base(new T[size + Alignment]),
	_data(align(_base.get(), size + Alignment, size, Alignment)),
	_size(size)
{}

template<typename T, std::size_t Alignment>
T* aligned_array<T, Alignment>::data()
{
	return _data;
}

template<typename T, std::size_t Alignment>
T const* aligned_array<T, Alignment>::data() const
{
	return _data;
}

template<typename T, std::size_t Alignment>
std::size_t aligned_array<T, Alignment>::size() const
{
	return _size;
}

template<typename T, std::size_t Alignment>
T& aligned_array<T, Alignment>::operator[](std::size_t idx)
{
	debug_assert(idx < _size);
	return _data[idx];
}

template<typename T, std::size_t Alignment>
T const& aligned_array<T, Alignment>::operator[](std::size_t idx) const
{
	debug_assert(idx < _size);
	return _data[idx];
}


template<typename T>
T* align(T* buf, std::size_t actual_size, std::size_t required_size, std::size_t alignment)
{
	void* buf_v = buf;
	auto new_buf =  static_cast<T*>(std::align(alignment, required_size, buf_v, actual_size));
	if (new_buf == nullptr) {
		throw std::invalid_argument("Buffer too small.");
	}
	else {
		return new_buf;
	}
}

template<std::size_t NumPartitions>
constexpr std::array<std::pair<std::size_t, std::size_t>, NumPartitions> partition(std::size_t val)
{
	std::array<std::pair<std::size_t, std::size_t>, NumPartitions> res{};
	std::size_t const div = val / NumPartitions;
	std::size_t mod = val % NumPartitions;

	std::size_t offset = 0;
	for (std::size_t i = 0; i < NumPartitions; ++i) {
		res[i].first = offset;
		if (offset < val) {
			std::size_t const extra = mod > 0;
			std::size_t const count = div + extra;
			res[i].second = count;
			offset += count;
			mod -= extra;
		}
		else {
			res[i].second = 0;
		}
	}

	return res;
}
