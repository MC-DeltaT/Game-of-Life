#pragma once

#include "grid.hpp"
#include "utility.hpp"

#include <cstddef>
#include <cstdint>
#include <immintrin.h>
#include <intrin.h>


template<std::size_t Rows, std::size_t Cols>
state_updater<Rows, Cols>::state_updater(game_grid<Rows, Cols>& grid) :
	_grid(&grid),
	_neighbour_lut(Rows * Cols),
	_neighbours(Rows * Cols)
{
	for (std::ptrdiff_t grid_idx = 0; grid_idx < Rows * Cols; ++grid_idx) {
		for (std::size_t neighbour_idx = 0; neighbour_idx < _neighbour_offsets.size(); ++neighbour_idx) {
			std::ptrdiff_t const offset = _neighbour_offsets[neighbour_idx];
			_neighbour_lut[grid_idx][neighbour_idx] = wrap(grid_idx + offset, Rows * Cols);
		}
	}
}

template<std::size_t Rows, std::size_t Cols>
void state_updater<Rows, Cols>::precomp(std::size_t begin_idx, std::size_t end_idx)
{
	debug_assert(end_idx >= begin_idx);

	for (std::size_t grid_idx = begin_idx; grid_idx < end_idx; ++grid_idx) {
		_neighbours[grid_idx] = _count_neighbours(grid_idx);
	}
}

template<std::size_t Rows, std::size_t Cols>
void state_updater<Rows, Cols>::update(std::size_t begin_idx, std::size_t end_idx)
{
	debug_assert(end_idx >= begin_idx);

	std::size_t grid_idx = begin_idx;
	for (; grid_idx + _vectorised_count < end_idx; grid_idx += _vectorised_count) {
		_vectorised_update(grid_idx);
	}
	for (; grid_idx < end_idx; ++grid_idx) {
		_single_update(grid_idx);
	}
}

template<std::size_t Rows, std::size_t Cols>
std::uint8_t state_updater<Rows, Cols>::_count_neighbours(std::size_t grid_idx) const
{
	std::uint8_t neighbours = 0;
	for (auto const idx : _neighbour_lut[grid_idx]) {
		neighbours += _grid->get_curr(idx);
	}
	return neighbours;
}

template<std::size_t Rows, std::size_t Cols>
void state_updater<Rows, Cols>::_single_update(std::size_t grid_idx)
{
	bool const curr_state = _grid->get_curr(grid_idx);
	std::uint8_t const neighbours = _neighbours[grid_idx];

	bool new_state;
	if (neighbours == 2) {
		new_state = curr_state;
	}
	else {
		new_state = neighbours == 3;
	}

	_grid->set_next(grid_idx, new_state);
}

template<std::size_t Rows, std::size_t Cols>
void state_updater<Rows, Cols>::_vectorised_update(std::size_t grid_idx)
{
	// new_state = (neighbours == 2) ? state : (neighbours == 3)

	__m256i const neighbours_vec = _mm256_load_si256(reinterpret_cast<__m256i const*>(_neighbours.data() + grid_idx));
	__m256i const states_vec = _mm256_load_si256(reinterpret_cast<__m256i const*>(_grid->curr() + grid_idx));

	__m256i const neighbours_eq2 = _mm256_cmpeq_epi8(neighbours_vec, _mm256_set1_epi8(2u));
	__m256i const neighbours_eq3 = _mm256_cmpeq_epi8(neighbours_vec, _mm256_set1_epi8(3u));
	
	__m256i new_states_vec = _mm256_blendv_epi8(neighbours_eq3, states_vec, neighbours_eq2);
	new_states_vec = _mm256_and_si256(new_states_vec, _mm256_set1_epi8(1u));

	_mm256_store_si256(reinterpret_cast<__m256i*>(_grid->next() + grid_idx), new_states_vec);
}
