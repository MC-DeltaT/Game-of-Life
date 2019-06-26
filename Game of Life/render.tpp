#pragma once

#include "grid.hpp"
#include "utility.hpp"

#include <cstddef>
#include <limits>
#define NOMINMAX
#include <Windows.h>


template<std::size_t Rows, std::size_t Cols>
console_renderer<Rows, Cols>::console_renderer(game_grid<Rows, Cols> const& grid, HANDLE console_handle) :
	_grid(&grid),
	_data(_buf_size),
	_console_handle(console_handle)
{
	for (std::size_t i = Cols; i < _buf_size; i += Cols + 1) {
		_data[i] = '\n';
	}
}

template<std::size_t Rows, std::size_t Cols>
void console_renderer<Rows, Cols>::render(std::size_t begin_idx, std::size_t end_idx)
{
	debug_assert(end_idx >= begin_idx);

	for (std::size_t grid_idx = begin_idx; grid_idx < end_idx; ++grid_idx) {
		std::size_t buf_idx = grid_idx + (grid_idx / Cols);
		_render(grid_idx, buf_idx);
	}
}

template<std::size_t Rows, std::size_t Cols>
void console_renderer<Rows, Cols>::draw() const
{
	SetConsoleCursorPosition(_console_handle, COORD{0, 0});
	DWORD written;
	static_assert(_buf_size < std::numeric_limits<DWORD>::max());
	debug_assert(WriteConsoleA(_console_handle, _data.data(), static_cast<DWORD>(_buf_size), &written, NULL));
	debug_assert(written == _buf_size);
}

template<std::size_t Rows, std::size_t Cols>
void console_renderer<Rows, Cols>::_render(std::size_t grid_idx, std::size_t buf_idx)
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
