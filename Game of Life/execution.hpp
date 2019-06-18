#pragma once

#include "utility.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <thread>
#include <utility>


template<class GameGrid, class StateUpdater, class Renderer, std::size_t NumThreads>
class cpu_executor {
public:
	~cpu_executor();
	cpu_executor(GameGrid& grid, StateUpdater& updater, Renderer& renderer);

	void render_and_update();

private:
	GameGrid* _grid;
	StateUpdater* _updater;
	Renderer* _renderer;
	std::array<std::thread, NumThreads - 1> _threads;
	static inline constexpr std::array<std::pair<std::size_t, std::size_t>, NumThreads> _partitions = partition<NumThreads>(GameGrid::size);
	std::atomic_size_t _sync1;
	std::atomic_size_t _sync2;
	bool _exit;

	void _render_and_update(std::size_t thread_idx);
	void _thread_func(std::size_t thread_idx);
};

template<class GameGrid, class StateUpdater, class Renderer>
class cpu_executor<GameGrid, StateUpdater, Renderer, 0> {};

template<class GameGrid, class StateUpdater, class Renderer>
class cpu_executor<GameGrid, StateUpdater, Renderer, 1> {
public:
	cpu_executor(GameGrid& grid, StateUpdater& updater, Renderer& renderer);

	void render_and_update();

private:
	GameGrid* _grid;
	StateUpdater* _updater;
	Renderer* _renderer;
};


#include "execution.tpp"
