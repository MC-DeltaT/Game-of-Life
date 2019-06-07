#include "console.hpp"
#include "grid.hpp"
#include "utility.hpp"

#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <ratio>
#include <thread>
#include <vector>
#define NOMINMAX
#include <Windows.h>


#define BENCHMARK
#define BENCHMARK_ROWS 100
#define BENCHMARK_COLS 100
#define BENCHMARK_ITERATIONS 1000000ull


void update_thread(bool& exit, thread_barrier& barrier, game_grid& grid, std::size_t offset, std::size_t count)
{
	while (!exit) {
		barrier.wait();
		if (exit) {
			break;
		}
		for (std::size_t i = offset; i < offset + count; ++i) {
			grid.update_next(i);
		}
	}
}


int main()
{
	constexpr std::size_t num_threads = 3;

#ifdef BENCHMARK
	constexpr std::size_t rows = BENCHMARK_ROWS;
	constexpr std::size_t cols = BENCHMARK_COLS;

	std::cout << "Rows: " << rows << std::endl;
	std::cout << "Columns: " << cols << std::endl;
	std::cout << "Cells: " << rows * cols << std::endl;
	std::cout << "Iterations: " << BENCHMARK_ITERATIONS << std::endl;
	std::cout << "Total cell updates: " << rows * cols * BENCHMARK_ITERATIONS << std::endl;
	std::cout << "Threads: " << num_threads << std::endl;
#else
	HANDLE const stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	auto const [rows, cols] = get_console_size(stdout_handle);
	grid_console_display display(rows, cols);
#endif

	game_grid grid(rows, cols);
	grid.rand_init();

	auto partitions = partition(rows * cols, num_threads);
	std::vector<std::thread> threads;
	threads.reserve(partitions.size());
	thread_barrier barrier;
	bool exit = false;
	for (auto const& p : partitions) {
		threads.emplace_back(update_thread, std::ref(exit), std::ref(barrier), std::ref(grid), p.first, p.second);
	}

	barrier.wait_for(threads.size());

#ifdef BENCHMARK
	auto const t1 = std::chrono::high_resolution_clock::now();
	for (std::size_t n = 0; n < BENCHMARK_ITERATIONS; ++n) {
#else
	while (true) {
#endif
		barrier.release();

#ifndef BENCHMARK
		display.load(grid);
		display.display(stdout_handle);
#endif

		barrier.wait_for(threads.size());
		grid.load_next();
	}

#ifdef BENCHMARK
	auto const t2 = std::chrono::high_resolution_clock::now();
	auto const total_real_time = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(t2 - t1);
	auto const real_time_per_iter = total_real_time / BENCHMARK_ITERATIONS;
	auto const real_time_per_cell = real_time_per_iter / (rows * cols);
	auto const total_cpu_time = total_real_time * num_threads;
	auto const cpu_time_per_iter = total_cpu_time / BENCHMARK_ITERATIONS;
	auto const cpu_time_per_cell = cpu_time_per_iter / (rows * cols);

	std::cout << "Total real time: " << total_real_time.count() << "s" << std::endl;
	std::cout << "Total CPU time: " << total_cpu_time.count() << "s" << std::endl;
	std::cout << "Iterations per real second: " << 1.0 / real_time_per_iter.count() << std::endl;
	std::cout << "Iterations per CPU second: " << 1.0 / cpu_time_per_iter.count() << std::endl;
	std::cout << "Cell updates per real second: " << 1.0 / real_time_per_cell.count() << std::endl;
	std::cout << "Cell updates per CPU second: " << 1.0 / cpu_time_per_cell.count() << std::endl;
	std::cout << "Real time per cell update: " <<
		std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(real_time_per_cell).count() << "ns" << std::endl;
	std::cout << "CPU time per cell update: " <<
		std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(cpu_time_per_cell).count() << "ns" << std::endl;
#endif

	exit = true;
	barrier.release();
	
	for (auto& t : threads) {
		t.join();
	}
}
