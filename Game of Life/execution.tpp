#pragma once

#include "utility.hpp"

#include <cstddef>
#include <thread>


template<class GameGrid, class StateUpdater, class Renderer, std::size_t NumThreads>
cpu_executor<GameGrid, StateUpdater, Renderer, NumThreads>::~cpu_executor()
{
	_sync1 = 0;
	while (_sync2 != _threads.size()) {}
	_exit = true;
	_sync2 = 0;
	
	for (auto& t : _threads) {
		t.join();
	}
}

template<class GameGrid, class StateUpdater, class Renderer, std::size_t NumThreads>
cpu_executor<GameGrid, StateUpdater, Renderer, NumThreads>::cpu_executor(GameGrid& grid, StateUpdater& updater, Renderer& renderer) :
	_grid(&grid),
	_updater(&updater),
	_renderer(&renderer),
	_sync1(0),
	_sync2(0),
	_exit(false)
{
	for (std::size_t i = 0; i < NumThreads - 1; ++i) {
		_threads[i] = std::thread(&cpu_executor::_thread_func, this, i + 1);
	}
	while (_sync1 != _threads.size()) {}
}

template<class GameGrid, class StateUpdater, class Renderer, std::size_t NumThreads>
void cpu_executor<GameGrid, StateUpdater, Renderer, NumThreads>::render_and_update()
{
	_sync1 = 0;
	while (_sync2 != _threads.size()) {}
	_sync2 = 0;

	_render_and_update(0);

	while (_sync1 != _threads.size()) {}
}

template<class GameGrid, class StateUpdater, class Renderer, std::size_t NumThreads>
void cpu_executor<GameGrid, StateUpdater, Renderer, NumThreads>::_render_and_update(std::size_t thread_idx)
{
	auto const& partition = _partitions[thread_idx];
	auto const begin = partition.first;
	auto const end = partition.first + partition.second;

	for (std::size_t i = begin; i != end; ++i) {
		_renderer->render(i);
	}
	for (std::size_t i = begin; i != end; ++i) {
		_updater->update(i);
	}
}

template<class GameGrid, class StateUpdater, class Renderer, std::size_t NumThreads>
void cpu_executor<GameGrid, StateUpdater, Renderer, NumThreads>::_thread_func(std::size_t thread_idx)
{
	debug_assert(thread_idx >= 1);

	while (true) {
		++_sync1;
		while (_sync1 != 0) {}
		++_sync2;
		while (_sync2 != 0) {}

		if (_exit) {
			break;
		}

		_render_and_update(thread_idx);
	}
}


template<class GameGrid, class StateUpdater, class Renderer>
cpu_executor<GameGrid, StateUpdater, Renderer, 1>::cpu_executor(GameGrid& grid, StateUpdater& updater, Renderer& renderer) :
	_grid(&grid),
	_updater(&updater),
	_renderer(&renderer)
{}

template<class GameGrid, class StateUpdater, class Renderer>
void cpu_executor<GameGrid, StateUpdater, Renderer, 1>::render_and_update()
{
	_renderer->render_all();
	for (std::size_t i = 0; i != GameGrid::size; ++i) {
		_updater->update(i);
	}
}
