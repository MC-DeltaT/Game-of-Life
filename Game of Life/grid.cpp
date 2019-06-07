#include "grid.hpp"

#include "utility.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <limits>
#include <random>
#include <utility>


game_grid::game_grid(std::size_t rows, std::size_t cols) :
	_rows(rows),
	_cols(cols),
	_curr(new bool[rows * cols]),
	_next(new bool[rows * cols]),
	_neighbour_lut(rows* cols)
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

		return std::pair{wrap(i, _rows), wrap(j, _cols)};
	};

	auto to_idx = [this](std::size_t i, std::size_t j) {
		return (i * _cols) + j;
	};

	debug_assert(_rows < std::numeric_limits<std::ptrdiff_t>::max());
	debug_assert(_cols < std::numeric_limits<std::ptrdiff_t>::max());
	for (std::ptrdiff_t i = 0; i < _rows; ++i) {
		for (std::ptrdiff_t j = 0; j < _cols; ++j) {
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

std::size_t game_grid::rows() const
{
	return _rows;
}

std::size_t game_grid::cols() const
{
	return _cols;
}

void game_grid::rand_init()
{
	static std::default_random_engine rand_eng(std::chrono::system_clock::now().time_since_epoch().count());
	std::generate_n(_curr.get(), _rows * _cols, []() {return rand_eng() & 1u; });
}

bool game_grid::curr_state(std::size_t i) const
{
	return _curr[i];
}

bool game_grid::update_next(std::size_t i)
{
	bool const next = _next_state(i);
	_next[i] = next;
	return next;
}

void game_grid::load_next()
{
	swap(_curr, _next);
}


unsigned game_grid::_count_neighbours(std::size_t i) const
{
	unsigned neighbours = 0;
	for (auto offset : _neighbour_lut[i]) {
		neighbours += _curr[offset];
	}
	return neighbours;
}

bool game_grid::_next_state(std::size_t i) const
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
