#pragma once

#include "grid.hpp"

#include <cstddef>
#include <vector>
#define NOMINMAX
#include <Windows.h>


template<std::size_t Rows, std::size_t Cols>
class console_renderer {
public:
	console_renderer(game_grid<Rows, Cols> const& grid, HANDLE console_handle);

	void render(std::size_t begin_idx, std::size_t end_idx);
	void draw() const;

private:
	static inline constexpr char live_cell = 'x';
	static inline constexpr char dead_cell = ' ';
	static inline constexpr std::size_t _buf_size = Rows * (Cols + 1);

	game_grid<Rows, Cols> const* _grid;
	std::vector<char> _data;
	HANDLE _console_handle;

	void _render(std::size_t grid_idx, std::size_t buf_idx);
};

class null_renderer {
public:
	template<typename... Args>
	void render(Args&&...) const {}
	template<typename... Args>
	void draw(Args&&...) const {}
};


#include "render.tpp"
