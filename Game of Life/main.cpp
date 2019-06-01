#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <thread>
#include <utility>
#include <vector>
#define NOMINMAX
#include <Windows.h>


#define BENCHMARK
#define BENCHMARK_ROWS 100
#define BENCHMARK_COLS 100
#define BENCHMARK_ITERATIONS 1000000ull


__forceinline void debug_assert(bool b)
{
#ifdef _DEBUG
	assert(b);
#endif
}


class game_grid {
public:
	game_grid(std::size_t rows, std::size_t cols) :
		_rows(rows),
		_cols(cols),
		_curr(new bool[rows * cols]),
		_next(new bool[rows * cols]),
		_neighbour_lut(rows * cols)
	{
		auto wrap = [this](std::ptrdiff_t i, std::ptrdiff_t j) {
			auto wrap = [](std::ptrdiff_t x, std::size_t bound) {
				std::size_t res;
				if (x < 0) {
					res = x + bound;
				}
				else if (x >= bound) {
					res = x % bound;
				}
				else {
					res = x;
				}
				debug_assert(res < bound);
				return res;
			};

			return std::pair{wrap(i, _rows), wrap(j, _cols)};
		};

		auto to_idx = [this](std::size_t i, std::size_t j) {
			return (i * _cols) + j;
		};

		debug_assert(_rows < std::numeric_limits<std::ptrdiff_t>::max());
		debug_assert(_cols < std::numeric_limits<std::ptrdiff_t>::max());
		for (std::ptrdiff_t i = 0; i < _rows; ++i) {
			for (std::ptrdiff_t j = 0; j < _cols; ++j) {
				std::size_t const cell_idx = to_idx(i, j);
				for (std::size_t n = 0; n < _neighbour_offsets.size(); ++n) {
					auto const [offset_i, offset_j] = _neighbour_offsets[n];
					auto const [neighbour_i, neighbour_j] = wrap(i + offset_i, j + offset_j);
					std::size_t const neighbour_idx = to_idx(neighbour_i, neighbour_j);
					_neighbour_lut[cell_idx][n] = neighbour_idx;
				}
			}
		}
	}

	std::size_t rows() const
	{
		return _rows;
	}

	std::size_t cols() const
	{
		return _cols;
	}

	void rand_init()
	{
		static std::default_random_engine rand_eng(std::chrono::system_clock::now().time_since_epoch().count());
		std::generate_n(_curr.get(), _rows * _cols, []() {return rand_eng() & 1u;});
	}

	bool curr_state(std::size_t i) const
	{
		return _curr[i];
	}

	bool update_next(std::size_t i)
	{
		bool const next = _next_state(i);
		_next[i] = next;
		return next;
	}

	void load_next()
	{
		swap(_curr, _next);
	}

private:
	static inline constexpr std::array<std::pair<std::ptrdiff_t, std::ptrdiff_t>, 8> _neighbour_offsets{
		std::pair{-1, -1}, std::pair{-1, 0}, std::pair{-1, 1},
		std::pair{0, -1},					 std::pair{0, 1},
		std::pair{1, -1},  std::pair{1, 0},  std::pair{1, 1}
	};

	std::size_t _rows;
	std::size_t _cols;
	std::unique_ptr<bool[]> _curr;
	std::unique_ptr<bool[]> _next;
	std::vector<std::array<std::size_t, _neighbour_offsets.size()>> _neighbour_lut;
	
	unsigned _count_neighbours(std::size_t i) const
	{
		unsigned neighbours = 0;
		for (auto offset : _neighbour_lut[i]) {
			neighbours += _curr[offset];
		}
		return neighbours;
	}

	bool _next_state(std::size_t i) const
	{
		unsigned const neighbours = _count_neighbours(i);
		bool const curr_state = _curr[i];
		bool new_state;

		if (curr_state) {
			if (neighbours < 2) {
				new_state = false;
			}
			else if (neighbours <= 3) {
				new_state = true;
			}
			else {
				new_state = false;
			}
		}
		else {
			if (neighbours == 3) {
				new_state = true;
			}
			else {
				new_state = false;
			}
		}

		return new_state;
	}
};


class grid_console_display {
public:
	grid_console_display(std::size_t rows, std::size_t cols) :
		_data(rows * (cols + 1)),
		_rows(rows),
		_cols(cols),
		_grid_size(rows * cols),
		_buf_size(rows * (cols + 1))
	{
		for (std::size_t i = _cols; i < _buf_size; i += _cols + 1) {
			_data[i] = '\n';
		}
	}

	void load(game_grid const& grid)
	{
		debug_assert(grid.rows() == _rows);
		debug_assert(grid.cols() == _cols);
		for (std::size_t i = 0; i < _rows; ++i) {
			for (std::size_t j = 0; j < _cols; ++j) {
				bool const state = grid.curr_state((i * _cols) + j);
				if (state) {
					_data[(i * (_cols + 1)) + j] = live_cell;
				}
				else {
					_data[(i * (_cols + 1)) + j] = dead_cell;
				}
			}
		}
	}

	void display(HANDLE console_handle)
	{
		SetConsoleCursorPosition(console_handle, COORD{0, 0});
		DWORD written;
		debug_assert(_data.size() < std::numeric_limits<DWORD>::max());
		WriteConsoleA(console_handle, _data.data(), _buf_size, &written, NULL);
		debug_assert(written == _buf_size);
	}

private:
	static inline constexpr char live_cell = 'x';
	static inline constexpr char dead_cell = ' ';

	std::vector<char> _data;
	std::size_t _rows;
	std::size_t _cols;
	std::size_t _grid_size;
	std::size_t _buf_size;
};


struct thread_barrier {
	std::atomic_size_t count1 = 0;
	std::atomic_size_t count2 = 0;

	void wait()
	{
		++count1;
		while (count1 > 0) {}
		++count2;
		while (count2 > 0) {}
	}

	void wait_for(std::size_t count)
	{
		while (count1 < count) {}
		count1 = 0;
		while (count2 < count) {}
	}

	void release()
	{
		count2 = 0;
	}

	void sync_with(std::size_t count)
	{
		wait_for(count);
		release();
	}
};


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


std::vector<std::pair<std::size_t, std::size_t>> partition(std::size_t val, std::size_t partitions)
{
	std::vector<std::pair<std::size_t, std::size_t>> res;
	if (partitions > 0) {
		std::size_t const actual_partitions = val < partitions ? val : partitions;
		std::size_t const div = val / actual_partitions;
		std::size_t mod = val % actual_partitions;

		res.reserve(actual_partitions);
		std::size_t offset = 0;
		for (std::size_t i = 0; i < actual_partitions; ++i) {
			std::size_t extra = mod > 0 ? 1 : 0;
			res.emplace_back(offset, div + extra);
			offset += div + extra;
			mod -= extra;
		}
	}

#ifdef _DEBUG
	std::size_t total = 0;
	std::size_t next_offset = 0;
	for (auto const& p : res) {
		debug_assert(p.first == next_offset);
		total += p.second;
		next_offset = p.first + p.second;
	}
	debug_assert(total == val);
#endif

	return res;
}


std::pair<std::size_t, std::size_t> get_console_size(HANDLE console_handle)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	debug_assert(GetConsoleScreenBufferInfo(console_handle, &csbi));
	return {csbi.srWindow.Bottom - csbi.srWindow.Top,
			csbi.srWindow.Right - csbi.srWindow.Left};
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
