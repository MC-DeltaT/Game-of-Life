#pragma once

#include "grid.hpp"
#include "utility.hpp"

#include <cstddef>


template<class Renderer>
single_thread_executor<Renderer>::single_thread_executor(game_grid& grid, Renderer& renderer) :
	_grid(&grid),
	_renderer(&renderer)
{}

template<class Renderer>
void single_thread_executor<Renderer>::render_and_update()
{
	_renderer->render_all();
	for (std::size_t i = 0; i < _grid->size(); ++i) {
		_grid->update_next(i);
	}
}


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
		_sync.wait_for(_threads.size());
	}
}

template<class Renderer>
void multi_thread_executor<Renderer>::render_and_update()
{
	if (_threads.size() >= 1) {
		_sync.sync();
	}
	_render_and_update(0);
	if (_threads.size() >= 1) {
		_sync.wait_for(_threads.size());
	}
}

template<class Renderer>
void multi_thread_executor<Renderer>::_render_and_update(std::size_t thread_idx)
{
	thread_local auto const& partition = _partitions[thread_idx];
	thread_local auto const begin = partition.first;
	thread_local auto const end = partition.first + partition.second;

	for (std::size_t i = begin; i != end; ++i) {
		_renderer->render(i);
	}
	for (std::size_t i = begin; i != end; ++i) {
		_grid->update_next(i);
	}
}

template<class Renderer>
void multi_thread_executor<Renderer>::_thread_func(std::size_t thread_idx)
{
	debug_assert(thread_idx >= 1);
	thread_sync sync(_sync);
	while (true) {
		sync.sync();
		if (_exit) {
			break;
		}
		_render_and_update(thread_idx);
	}
}
