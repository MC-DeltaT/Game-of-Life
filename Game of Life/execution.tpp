#pragma once

#include <array>
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
	_start_threads<0>(_threads);
	while (_sync1 != _threads.size()) {}
}

template<class GameGrid, class StateUpdater, class Renderer, std::size_t NumThreads>
void cpu_executor<GameGrid, StateUpdater, Renderer, NumThreads>::render_and_update()
{
	_sync1 = 0;
	while (_sync2 != _threads.size()) {}
	_sync2 = 0;

	_render_and_update<0>();

	while (_sync1 != _threads.size()) {}
}

template<class GameGrid, class StateUpdater, class Renderer, std::size_t NumThreads>
template<std::size_t ThreadIdx>
void cpu_executor<GameGrid, StateUpdater, Renderer, NumThreads>::_render_and_update()
{
	constexpr auto& partition = std::get<ThreadIdx>(_partitions);
	constexpr auto begin = partition.first;
	constexpr auto end = partition.first + partition.second;

	for (std::size_t i = begin; i != end; ++i) {
		_renderer->render(i);
	}
	for (std::size_t i = begin; i != end; ++i) {
		_updater->update(i);
	}
}

template<class GameGrid, class StateUpdater, class Renderer, std::size_t NumThreads>
template<std::size_t Idx>
void cpu_executor<GameGrid, StateUpdater, Renderer, NumThreads>::_start_threads(std::array<std::thread, NumThreads - 1>& threads)
{
	if constexpr (Idx < NumThreads - 1) {
		std::get<Idx>(threads) = std::thread(&cpu_executor::_thread_func<Idx + 1>, this);
		_start_threads<Idx + 1>(threads);
	}
}

template<class GameGrid, class StateUpdater, class Renderer, std::size_t NumThreads>
template<std::size_t ThreadIdx>
void cpu_executor<GameGrid, StateUpdater, Renderer, NumThreads>::_thread_func()
{
	static_assert(ThreadIdx >= 1);

	while (true) {
		++_sync1;
		while (_sync1 != 0) {}
		++_sync2;
		while (_sync2 != 0) {}

		if (_exit) {
			break;
		}

		_render_and_update<ThreadIdx>();
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
