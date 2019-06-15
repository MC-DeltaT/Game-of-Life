#pragma once

#include "utility.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <limits>
#include <random>
#include <utility>


template<std::size_t Rows, std::size_t Cols>
game_grid<Rows, Cols>::game_grid() :
	_curr(new bool[Rows * Cols]{}),
	_next(new bool[Rows * Cols]{}),
	_neighbour_lut(Rows * Cols)
{
	auto wrap = [this](std::ptrdiff_t i, std::ptrdiff_t j) {
		auto wrap = [](std::ptrdiff_t x, std::size_t bound) {
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
		};

		return std::pair{wrap(i, Rows), wrap(j, Cols)};
	};

	auto to_idx = [this](std::size_t i, std::size_t j) {
		return (i * Cols) + j;
	};

	static_assert(Rows < std::numeric_limits<std::ptrdiff_t>::max());
	static_assert(Cols < std::numeric_limits<std::ptrdiff_t>::max());
	for (std::ptrdiff_t i = 0; i < Rows; ++i) {
		for (std::ptrdiff_t j = 0; j < Cols; ++j) {
			std::size_t const cell_idx = to_idx(i, j);
			for (std::size_t n = 0; n < _neighbour_offsets.size(); ++n) {
				auto const [offset_i, offset_j] = _neighbour_offsets[n];
				auto const [neighbour_i, neighbour_j] = wrap(i + offset_i, j + offset_j);
				std::size_t const neighbour_idx = to_idx(neighbour_i, neighbour_j);
				_neighbour_lut[cell_idx][n] = neighbour_idx;
			}
		}
	}
}

template<std::size_t Rows, std::size_t Cols>
constexpr std::size_t game_grid<Rows, Cols>::rows() const
{
	return Rows;
}

template<std::size_t Rows, std::size_t Cols>
constexpr std::size_t game_grid<Rows, Cols>::cols() const
{
	return Cols;
}

template<std::size_t Rows, std::size_t Cols>
constexpr std::size_t game_grid<Rows, Cols>::size() const
{
	return Rows * Cols;
}

template<std::size_t Rows, std::size_t Cols>
void game_grid<Rows, Cols>::rand_init()
{
	static std::default_random_engine rand_eng(std::chrono::system_clock::now().time_since_epoch().count());
	std::generate_n(_curr.get(), Rows * Cols, []() { return rand_eng() & 1u; });
}

template<std::size_t Rows, std::size_t Cols>
bool game_grid<Rows, Cols>::curr_state(std::size_t i) const
{
	return _curr[i];
}

template<std::size_t Rows, std::size_t Cols>
bool game_grid<Rows, Cols>::update_next(std::size_t i)
{
	bool const next = _next_state(i);
	_next[i] = next;
	return next;
}

template<std::size_t Rows, std::size_t Cols>
void game_grid<Rows, Cols>::load_next()
{
	swap(_curr, _next);
}


template<std::size_t Rows, std::size_t Cols>
unsigned game_grid<Rows, Cols>::_count_neighbours(std::size_t i) const
{
	unsigned neighbours = 0;
	for (auto offset : _neighbour_lut[i]) {
		neighbours += _curr[offset];
	}
	return neighbours;
}

template<std::size_t Rows, std::size_t Cols>
bool game_grid<Rows, Cols>::_next_state(std::size_t i) const
{
	unsigned const neighbours = _count_neighbours(i);
	bool const curr_state = _curr[i];
	bool new_state;

	if (curr_state) {
		if (neighbours < 2) {
			new_state = false;
		}
		else if (neighbours <= 3) {
			new_state = true;
		}
		else {
			new_state = false;
		}
	}
	else {
		if (neighbours == 3) {
			new_state = true;
		}
		else {
			new_state = false;
		}
	}

	return new_state;
}
