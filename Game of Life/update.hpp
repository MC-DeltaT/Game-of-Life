#pragma once

#include "grid.hpp"
#include "utility.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>


template<std::size_t Rows, std::size_t Cols>
class state_updater {
public:
	state_updater(game_grid<Rows, Cols>& grid);

	void precomp(std::size_t begin_idx, std::size_t end_idx);
	void update(std::size_t begin_idx, std::size_t end_idx);

private:
	static inline constexpr std::size_t _vectorised_count = 256 / 8;
	static inline constexpr std::size_t _neighbours_align = _vectorised_count;
	static inline constexpr std::array<std::ptrdiff_t, 8> _neighbour_offsets{
		-std::ptrdiff_t{Cols} - 1, -std::ptrdiff_t{Cols}, -std::ptrdiff_t{Cols} + 1,
		-1,												  1,
		Cols - 1,				   Cols,				  Cols + 1
	};

	game_grid<Rows, Cols>* _grid;
	std::vector<std::array<std::size_t, _neighbour_offsets.size()>> _neighbour_lut;
	aligned_array<std::uint8_t, _neighbours_align> _neighbours;

	std::uint8_t _count_neighbours(std::size_t grid_idx) const;
	void _single_update(std::size_t grid_idx);
	void _vectorised_update(std::size_t grid_idx);
};


#include "update.tpp"
