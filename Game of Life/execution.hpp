#pragma once

#include "grid.hpp"
#include "utility.hpp"

#include <array>
#include <cstddef>
#include <thread>
#include <utility>


template<class Renderer, std::size_t NumThreads>
class cpu_executor {
public:
	~cpu_executor();
	cpu_executor(game_grid& grid, Renderer& renderer);

	void render_and_update();

private:
	game_grid* _grid;
	Renderer* _renderer;
	std::array<std::thread, NumThreads - 1> _threads;
	std::array<std::pair<std::size_t, std::size_t>, NumThreads> _partitions;
	thread_sync _sync;
	bool _exit;

	void _render_and_update(std::size_t thread_idx);
	void _thread_func(std::size_t thread_idx);
};

template<class Renderer>
class cpu_executor<Renderer, 0> {};

template<class Renderer>
class cpu_executor<Renderer, 1> {
public:
	cpu_executor(game_grid& grid, Renderer& renderer);

	void render_and_update();

private:
	game_grid* _grid;
	Renderer* _renderer;
};


#include "execution.tpp"
