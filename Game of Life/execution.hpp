#pragma once

#include "grid.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <thread>
#include <utility>


template<std::size_t Rows, std::size_t Cols, class Renderer, std::size_t NumThreads>
class cpu_executor {
public:
	~cpu_executor();
	cpu_executor(game_grid<Rows, Cols>& grid, Renderer& renderer);

	void render_and_update();

private:
	game_grid<Rows, Cols>* _grid;
	Renderer* _renderer;
	std::array<std::thread, NumThreads - 1> _threads;
	std::array<std::pair<std::size_t, std::size_t>, NumThreads> _partitions;
	std::atomic_size_t _sync1;
	std::atomic_size_t _sync2;
	bool _exit;

	void _render_and_update(std::size_t thread_idx);
	void _thread_func(std::size_t thread_idx);
};

template<std::size_t Rows, std::size_t Cols, class Renderer>
class cpu_executor<Rows, Cols, Renderer, 0> {};

template<std::size_t Rows, std::size_t Cols, class Renderer>
class cpu_executor<Rows, Cols, Renderer, 1> {
public:
	cpu_executor(game_grid<Rows, Cols>& grid, Renderer& renderer);

	void render_and_update();

private:
	game_grid<Rows, Cols>* _grid;
	Renderer* _renderer;
};


#include "execution.tpp"
