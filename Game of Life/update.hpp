#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>


template<class GameGrid>
class state_updater {
public:
	state_updater(GameGrid& grid);

	void update(std::size_t grid_idx);

private:
	GameGrid* _grid;
	std::vector<std::array<std::size_t, GameGrid::neighbour_count>> _neighbour_lut;

	std::uint8_t _count_neighbours(std::size_t grid_idx) const;
	bool _next_state(bool state, std::size_t neighbours) const;
};


#include "update.tpp"
