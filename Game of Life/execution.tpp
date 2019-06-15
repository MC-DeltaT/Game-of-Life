#pragma once

#include "grid.hpp"
#include "utility.hpp"

#include <cstddef>
#include <thread>


template<std::size_t Rows, std::size_t Cols, class Renderer>
cpu_executor<Rows, Cols, Renderer, 1>::cpu_executor(game_grid<Rows, Cols>& grid, Renderer& renderer) :
	_grid(&grid),
	_renderer(&renderer)
{}

template<std::size_t Rows, std::size_t Cols, class Renderer>
void cpu_executor<Rows, Cols, Renderer, 1>::render_and_update()
{
	_renderer->render_all();
	for (std::size_t i = 0; i < _grid->size(); ++i) {
		_grid->update_next(i);
	}
}


template<std::size_t Rows, std::size_t Cols, class Renderer, std::size_t NumThreads>
cpu_executor<Rows, Cols, Renderer, NumThreads>::~cpu_executor()
{
	_sync1 = 0;
	while (_sync2 != _threads.size()) {}
	_exit = true;
	_sync2 = 0;
	
	for (auto& t : _threads) {
		t.join();
	}
}

template<std::size_t Rows, std::size_t Cols, class Renderer, std::size_t NumThreads>
cpu_executor<Rows, Cols, Renderer, NumThreads>::cpu_executor(game_grid<Rows, Cols>& grid, Renderer& renderer) :
	_grid(&grid),
	_renderer(&renderer),
	_partitions(partition<NumThreads>(grid.size())),
	_sync1(0),
	_sync2(0),
	_exit(false)
{
	for (std::size_t i = 0; i < NumThreads - 1; ++i) {
		_threads[i] = std::thread(&cpu_executor::_thread_func, this, i + 1);
	}
	while (_sync1 != _threads.size()) {}
}

template<std::size_t Rows, std::size_t Cols, class Renderer, std::size_t NumThreads>
void cpu_executor<Rows, Cols, Renderer, NumThreads>::render_and_update()
{
	_sync1 = 0;
	while (_sync2 != _threads.size()) {}
	_sync2 = 0;

	_render_and_update(0);

	while (_sync1 != _threads.size()) {}
}

template<std::size_t Rows, std::size_t Cols, class Renderer, std::size_t NumThreads>
void cpu_executor<Rows, Cols, Renderer, NumThreads>::_render_and_update(std::size_t thread_idx)
{
	auto const& partition = _partitions[thread_idx];
	auto const begin = partition.first;
	auto const end = partition.first + partition.second;

	for (std::size_t i = begin; i != end; ++i) {
		_renderer->render(i);
	}
	for (std::size_t i = begin; i != end; ++i) {
		_grid->update_next(i);
	}
}

template<std::size_t Rows, std::size_t Cols, class Renderer, std::size_t NumThreads>
void cpu_executor<Rows, Cols, Renderer, NumThreads>::_thread_func(std::size_t thread_idx)
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
