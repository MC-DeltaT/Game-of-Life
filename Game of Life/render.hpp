#pragma once

#include <cstddef>
#include <vector>
#define NOMINMAX
#include <Windows.h>


template<class GameGrid>
class console_renderer {
public:
	console_renderer(GameGrid const& grid, HANDLE console_handle);

	void render(std::size_t grid_idx);
	void render_all();
	void draw() const;

private:
	static inline constexpr char live_cell = 'x';
	static inline constexpr char dead_cell = ' ';
	static inline constexpr std::size_t _buf_size = GameGrid::size + GameGrid::rows;

	GameGrid const* _grid;
	std::vector<char> _data;
	HANDLE _console_handle;

	void _render(std::size_t grid_idx, std::size_t buf_idx);
};

class null_renderer {
public:
	template<typename... Args>
	void render(Args&&...) const {}
	template<typename... Args>
	void render_all(Args&&...) const {}
	template<typename... Args>
	void draw(Args&&...) const {}
};


#include "render.tpp"
