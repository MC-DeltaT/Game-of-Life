#include "execution.hpp"
#include "grid.hpp"
#include "render.hpp"
#include "utility.hpp"

#include <chrono>
#include <cstddef>
#include <iostream>
#include <ratio>
#define NOMINMAX
#include <Windows.h>


#define BENCHMARK
constexpr std::size_t benchmark_rows = 100;
constexpr std::size_t benchmark_cols = 100;
constexpr std::size_t benchmark_iterations = 1000000ull;


constexpr std::size_t num_threads = 3;


int main()
{
#ifdef BENCHMARK
	constexpr std::size_t rows = benchmark_rows;
	constexpr std::size_t cols = benchmark_cols;

	std::cout << "Rows: " << rows << std::endl;
	std::cout << "Columns: " << cols << std::endl;
	std::cout << "Cells: " << rows * cols << std::endl;
	std::cout << "Iterations: " << benchmark_iterations << std::endl;
	std::cout << "Total cell updates: " << rows * cols * benchmark_iterations << std::endl;
	std::cout << "Threads: " << num_threads << std::endl;
#else
	HANDLE const stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	auto const [rows, cols] = get_console_size(stdout_handle);
#endif

	game_grid grid(rows, cols);
	grid.rand_init();

#ifdef BENCHMARK
	null_renderer renderer{};
#else
	console_renderer renderer(grid, stdout_handle);
#endif

	cpu_executor<decltype(renderer), num_threads> executor(grid, renderer);

#ifdef BENCHMARK
	auto const t1 = std::chrono::high_resolution_clock::now();
	for (std::size_t n = 0; n < benchmark_iterations; ++n) {
#else
	while (true) {
#endif
		executor.render_and_update();
		renderer.draw();
		grid.load_next();
	}

#ifdef BENCHMARK
	auto const t2 = std::chrono::high_resolution_clock::now();
	auto const total_real_time = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(t2 - t1);
	auto const real_time_per_iter = total_real_time / benchmark_iterations;
	auto const real_time_per_cell = real_time_per_iter / (rows * cols);
	auto const total_cpu_time = total_real_time * num_threads;
	auto const cpu_time_per_iter = total_cpu_time / benchmark_iterations;
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
}
