#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <utility>
#include <random>


template<std::size_t Rows, std::size_t Cols>
game_grid<Rows, Cols>::game_grid() :
	_curr(new bool[size]{}),
	_next(new bool[size]{})
{}

template<std::size_t Rows, std::size_t Cols>
bool game_grid<Rows, Cols>::get_curr(std::size_t idx) const
{
	return _curr[idx];
}

template<std::size_t Rows, std::size_t Cols>
void game_grid<Rows, Cols>::set_next(std::size_t idx, bool state) const
{
	_next[idx] = state;
}

template<std::size_t Rows, std::size_t Cols>
void game_grid<Rows, Cols>::load_next()
{
	swap(_curr, _next);
}

template<std::size_t Rows, std::size_t Cols>
void game_grid<Rows, Cols>::rand_init()
{
	static std::default_random_engine rand_eng(std::chrono::system_clock::now().time_since_epoch().count());
	std::generate_n(_curr.get(), size, []() { return rand_eng() & 1u; });
}
