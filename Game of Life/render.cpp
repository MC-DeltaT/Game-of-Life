#include "render.hpp"

#include "grid.hpp"
#include "utility.hpp"

#include <cstddef>
#include <limits>
#define NOMINMAX
#include <Windows.h>


console_renderer::console_renderer(game_grid const& grid, HANDLE console_handle) :
	_grid(&grid),
	_data(grid.rows() * (grid.cols() + 1)),
	_buf_size(grid.rows() * (grid.cols() + 1)),
	_console_handle(console_handle)
{
	for (std::size_t i = _grid->cols(); i < _buf_size; i += _grid->cols() + 1) {
		_data[i] = '\n';
	}
}

void console_renderer::render(std::size_t grid_idx)
{
	std::size_t buf_idx = grid_idx + (grid_idx / _grid->cols());
	_render(grid_idx, buf_idx);
}

void console_renderer::render_all()
{
	std::size_t grid_idx = 0;
	std::size_t buf_idx = 0;
	for (std::size_t i = 0; i < _grid->rows(); ++i) {
		for (std::size_t j = 0; j < _grid->cols(); ++j) {
			_render(grid_idx, buf_idx);
			++grid_idx;
			++buf_idx;
		}
		++buf_idx;
	}
}

void console_renderer::draw() const
{
	SetConsoleCursorPosition(_console_handle, COORD{0, 0});
	DWORD written;
	debug_assert(_data.size() < std::numeric_limits<DWORD>::max());
	WriteConsoleA(_console_handle, _data.data(), static_cast<DWORD>(_buf_size), &written, NULL);
	debug_assert(written == _buf_size);
}

void console_renderer::_render(std::size_t grid_idx, std::size_t buf_idx)
{
	bool const state = _grid->curr_state(grid_idx);
	auto& c = _data[buf_idx];
	if (state) {
		c = live_cell;
	}
	else {
		c = dead_cell;
	}
}
