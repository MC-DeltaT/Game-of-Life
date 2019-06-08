#pragma once

#include "grid.hpp"

#include <cstddef>
#include <vector>
#define NOMINMAX
#include <Windows.h>


class console_renderer {
public:
	console_renderer(game_grid const& grid, HANDLE console_handle);

	void render(std::size_t grid_idx);
	void render_all();
	void draw() const;

private:
	static inline constexpr char live_cell = 'x';
	static inline constexpr char dead_cell = ' ';

	game_grid const* _grid;
	std::vector<char> _data;
	std::size_t _buf_size;
	HANDLE _console_handle;

	void _render(std::size_t grid_idx, std::size_t buf_idx);
};

class null_renderer {
public:
	void render(std::size_t) {}
	void render_all() {}
	void draw() {}
};