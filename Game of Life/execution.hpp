#pragma once

#include "grid.hpp"
#include "update.hpp"
#include "utility.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <thread>
#include <utility>


template<std::size_t Rows, std::size_t Cols, class Renderer, std::size_t NumThreads>
class cpu_executor {
public:
	~cpu_executor();
	cpu_executor(game_grid<Rows, Cols>& grid, state_updater<Rows, Cols>& updater, Renderer& renderer);

	void render_and_update();

private:
	game_grid<Rows, Cols>* _grid;
	state_updater<Rows, Cols>* _updater;
	Renderer* _renderer;
	std::array<std::thread, NumThreads - 1> _threads;
	static inline constexpr std::array<std::pair<std::size_t, std::size_t>, NumThreads> _partitions = partition<NumThreads>(Rows * Cols);
	std::atomic_size_t _sync1;
	std::atomic_size_t _sync2;
	bool _exit;

	template<std::size_t Idx>
	void _start_threads(std::array<std::thread, NumThreads - 1>& threads);
	template<std::size_t ThreadIdx>
	void _render_and_update();
	template<std::size_t ThreadIdx>
	void _thread_func();
};

template<std::size_t Rows, std::size_t Cols, class Renderer>
class cpu_executor<Rows, Cols, Renderer, 0> {};

template<std::size_t Rows, std::size_t Cols, class Renderer>
class cpu_executor<Rows, Cols, Renderer, 1> {
public:
	cpu_executor(game_grid<Rows, Cols>& grid, state_updater<Rows, Cols>& updater, Renderer& renderer);

	void render_and_update();

private:
	game_grid<Rows, Cols>* _grid;
	state_updater<Rows, Cols>* _updater;
	Renderer* _renderer;
};


#include "execution.tpp"
