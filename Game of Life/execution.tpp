#pragma once

#include "grid.hpp"
#include "utility.hpp"

#include <cstddef>
#include <thread>


template<class Renderer>
cpu_executor<Renderer, 1>::cpu_executor(game_grid& grid, Renderer& renderer) :
	_grid(&grid),
	_renderer(&renderer)
{}

template<class Renderer>
void cpu_executor<Renderer, 1>::render_and_update()
{
	_renderer->render_all();
	for (std::size_t i = 0; i < _grid->size(); ++i) {
		_grid->update_next(i);
	}
}


template<class Renderer, std::size_t NumThreads>
cpu_executor<Renderer, NumThreads>::~cpu_executor()
{
	_sync.sync([this]() { _exit = true; });
	
	for (auto& t : _threads) {
		t.join();
	}
}

template<class Renderer, std::size_t NumThreads>
cpu_executor<Renderer, NumThreads>::cpu_executor(game_grid& grid, Renderer& renderer) :
	_grid(&grid),
	_renderer(&renderer),
	_partitions(partition<NumThreads>(grid.size())),
	_exit(false)
{
	for (std::size_t i = 0; i < NumThreads - 1; ++i) {
		_threads[i] = std::thread(&cpu_executor::_thread_func, this, i + 1);
	}
	_sync.wait_for(NumThreads - 1);
}

template<class Renderer, std::size_t NumThreads>
void cpu_executor<Renderer, NumThreads>::render_and_update()
{
	_sync.sync();
	_render_and_update(0);
	_sync.wait_for(NumThreads - 1);
}

template<class Renderer, std::size_t NumThreads>
void cpu_executor<Renderer, NumThreads>::_render_and_update(std::size_t thread_idx)
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

template<class Renderer, std::size_t NumThreads>
void cpu_executor<Renderer, NumThreads>::_thread_func(std::size_t thread_idx)
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
