#pragma once

#include "utility.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>


template<class GameGrid>
state_updater<GameGrid>::state_updater(GameGrid& grid) :
	_grid(&grid),
	_neighbour_lut(GameGrid::size)
{
	for (std::ptrdiff_t grid_idx = 0; grid_idx < GameGrid::size; ++grid_idx) {
		for (std::size_t neighbour_idx = 0; neighbour_idx < GameGrid::neighbour_count; ++neighbour_idx) {
			std::ptrdiff_t const offset = GameGrid::neighbour_offsets[neighbour_idx];
			_neighbour_lut[grid_idx][neighbour_idx] = wrap(grid_idx + offset, GameGrid::size);
		}
	}
}

template<class GameGrid>
void state_updater<GameGrid>::update(std::size_t grid_idx)
{
	std::uint8_t const neighbours = _count_neighbours(grid_idx);
	bool const next_state = _next_state(_grid->get_curr(grid_idx), neighbours);
	_grid->set_next(grid_idx, next_state);
}

template<class GameGrid>
std::uint8_t state_updater<GameGrid>::_count_neighbours(std::size_t grid_idx) const
{
	static_assert(GameGrid::neighbour_count <= std::numeric_limits<std::uint8_t>::max());

	std::uint8_t neighbours = 0;
	for (auto idx : _neighbour_lut[grid_idx]) {
		neighbours += _grid->get_curr(idx);
	}
	return neighbours;
}

template<class GameGrid>
bool state_updater<GameGrid>::_next_state(bool state, std::size_t neighbours) const
{
	bool new_state;
	if (neighbours == 2) {
		new_state = state;
	}
	else {
		new_state = neighbours == 3;
	}
	return new_state;
}
