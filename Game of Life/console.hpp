#pragma once

#include "grid.hpp"

#include <cstddef>
#include <utility>
#include <vector>
#define NOMINMAX
#include <Windows.h>


class grid_console_display {
public:
	grid_console_display(std::size_t rows, std::size_t cols);

	void load(game_grid const& grid);
	void display(HANDLE console_handle);

private:
	static inline constexpr char live_cell = 'x';
	static inline constexpr char dead_cell = ' ';

	std::vector<char> _data;
	std::size_t _rows;
	std::size_t _cols;
	std::size_t _grid_size;
	std::size_t _buf_size;
};


std::pair<std::size_t, std::size_t> get_console_size(HANDLE console_handle);
