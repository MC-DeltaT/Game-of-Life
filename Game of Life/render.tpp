#pragma once

#include "utility.hpp"

#include <cstddef>
#include <limits>
#define NOMINMAX
#include <Windows.h>


template<class GameGrid>
console_renderer<GameGrid>::console_renderer(GameGrid const& grid, HANDLE console_handle) :
	_grid(&grid),
	_data(_buf_size),
	_console_handle(console_handle)
{
	for (std::size_t i = GameGrid::cols; i < _buf_size; i += GameGrid::cols + 1) {
		_data[i] = '\n';
	}
}

template<class GameGrid>
void console_renderer<GameGrid>::render(std::size_t grid_idx)
{
	std::size_t buf_idx = grid_idx + (grid_idx / GameGrid::cols);
	_render(grid_idx, buf_idx);
}

template<class GameGrid>
void console_renderer<GameGrid>::render_all()
{
	std::size_t grid_idx = 0;
	std::size_t buf_idx = 0;
	for (std::size_t i = 0; i < GameGrid::rows; ++i) {
		for (std::size_t j = 0; j < GameGrid::cols; ++j) {
			_render(grid_idx, buf_idx);
			++grid_idx;
			++buf_idx;
		}
		++buf_idx;
	}
}

template<class GameGrid>
void console_renderer<GameGrid>::draw() const
{
	SetConsoleCursorPosition(_console_handle, COORD{0, 0});
	DWORD written;
	static_assert(_buf_size < std::numeric_limits<DWORD>::max());
	debug_assert(WriteConsoleA(_console_handle, _data.data(), static_cast<DWORD>(_buf_size), &written, NULL));
	debug_assert(written == _buf_size);
}

template<class GameGrid>
void console_renderer<GameGrid>::_render(std::size_t grid_idx, std::size_t buf_idx)
{
	bool const state = _grid->get_curr(grid_idx);
	auto& c = _data[buf_idx];
	if (state) {
		c = live_cell;
	}
	else {
		c = dead_cell;
	}
}
