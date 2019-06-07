#include "console.hpp"

#include "grid.hpp"
#include "utility.hpp"

#include <cstddef>
#include <limits>
#define NOMINMAX
#include <Windows.h>


grid_console_display::grid_console_display(std::size_t rows, std::size_t cols) :
	_data(rows* (cols + 1)),
	_rows(rows),
	_cols(cols),
	_grid_size(rows* cols),
	_buf_size(rows* (cols + 1))
{
	for (std::size_t i = _cols; i < _buf_size; i += _cols + 1) {
		_data[i] = '\n';
	}
}

void grid_console_display::load(game_grid const& grid)
{
	debug_assert(grid.rows() == _rows);
	debug_assert(grid.cols() == _cols);
	std::size_t grid_idx = 0;
	std::size_t buf_idx = 0;
	for (std::size_t i = 0; i < _rows; ++i) {
		for (std::size_t j = 0; j < _cols; ++j) {
			bool const state = grid.curr_state(grid_idx);
			auto& c = _data[buf_idx];
			if (state) {
				c = live_cell;
			}
			else {
				c = dead_cell;
			}
			++grid_idx;
			++buf_idx;
		}
		++buf_idx;
	}
}

void grid_console_display::display(HANDLE console_handle)
{
	SetConsoleCursorPosition(console_handle, COORD{ 0, 0 });
	DWORD written;
	debug_assert(_data.size() < std::numeric_limits<DWORD>::max());
	WriteConsoleA(console_handle, _data.data(), static_cast<DWORD>(_buf_size), &written, NULL);
	debug_assert(written == _buf_size);
}


std::pair<std::size_t, std::size_t> get_console_size(HANDLE console_handle)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	debug_assert(GetConsoleScreenBufferInfo(console_handle, &csbi));
	return {csbi.srWindow.Bottom - csbi.srWindow.Top,
			csbi.srWindow.Right - csbi.srWindow.Left};
}
