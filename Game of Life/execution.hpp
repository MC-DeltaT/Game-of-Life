#pragma once

#include "grid.hpp"
#include "utility.hpp"

#include <cstddef>
#include <thread>
#include <utility>
#include <vector>


template<class Renderer>
class single_thread_executor {
public:
	single_thread_executor(game_grid& grid, Renderer& renderer);

	void update_and_render();

private:
	game_grid* _grid;
	Renderer* _renderer;
};

template<class Renderer>
class multi_thread_executor {
public:
	~multi_thread_executor();
	multi_thread_executor(std::size_t num_threads, game_grid& grid, Renderer& renderer);

	void update_and_render();

private:
	game_grid* _grid;
	Renderer* _renderer;
	std::vector<std::thread> _threads;
	std::vector<std::pair<std::size_t, std::size_t>> _partitions;
	thread_sync _sync;
	bool _exit;

	void _update_and_render(std::size_t thread_idx);
	void _thread_func(std::size_t thread_idx);
};


#include "execution.tpp"
