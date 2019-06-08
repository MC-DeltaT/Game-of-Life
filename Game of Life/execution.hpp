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
single_thread_executor<Renderer>::single_thread_executor(game_grid& grid, Renderer& renderer) :
	_grid(&grid),
	_renderer(&renderer)
{}

template<class Renderer>
void single_thread_executor<Renderer>::update_and_render()
{
	for (std::size_t i = 0; i < _grid->size(); ++i) {
		_grid->update_next(i);
	}
	_renderer->render_all();
}


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

template<class Renderer>
multi_thread_executor<Renderer>::~multi_thread_executor()
{
	if (_threads.size() >= 1) {
		_sync.sync([this]() { _exit = true; });
	}
	
	for (auto& t : _threads) {
		t.join();
	}
}

template<class Renderer>
multi_thread_executor<Renderer>::multi_thread_executor(std::size_t num_threads, game_grid& grid, Renderer& renderer) :
	_grid(&grid),
	_renderer(&renderer),
	_partitions(partition(grid.size(), num_threads)),
	_exit(false)
{
	debug_assert(num_threads >= 1);
	if (_partitions.size() >= 2) {
		_threads.reserve(_partitions.size() - 1);
		for (std::size_t i = 1; i < _partitions.size(); ++i) {
			_threads.emplace_back(&multi_thread_executor::_thread_func, this, i);
		}
		while (_sync.waiting() < _threads.size()) {}
	}
}

template<class Renderer>
void multi_thread_executor<Renderer>::update_and_render()
{
	if (_threads.size() >= 1) {
		_sync.sync();
	}
	_update_and_render(0);
	if (_threads.size() >= 1) {
		while (_sync.waiting() < _threads.size()) {}
	}
}

template<class Renderer>
void multi_thread_executor<Renderer>::_update_and_render(std::size_t thread_idx)
{
	auto const& partition = _partitions[thread_idx];
	for (std::size_t i = partition.first; i < partition.first + partition.second; ++i) {
		_grid->update_next(i);
		_renderer->render(i);
	}
}

template<class Renderer>
void multi_thread_executor<Renderer>::_thread_func(std::size_t thread_idx)
{
	debug_assert(thread_idx >= 1);
	thread_sync sync(_sync);
	while (!_exit) {
		sync.sync();
		if (_exit) {
			break;
		}
		_update_and_render(thread_idx);
	}
}
