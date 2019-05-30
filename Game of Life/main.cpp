#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
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


inline void debug_assert(bool b)
{
#ifdef _DEBUG
	assert(b);
#endif
}


template<typename T>
class matrix {
public:
	matrix(std::size_t rows, std::size_t cols) :
		_rows(rows), _cols(cols), _data(new T[rows * cols])
	{}

	T* data()
	{
		return _data.get();
	}

	T const* data() const
	{
		return _data.get();
	}

	std::size_t rows() const
	{
		return _rows;
	}

	std::size_t cols() const
	{
		return _cols;
	}

	std::size_t size() const
	{
		return _rows * _cols;
	}

	T const& get(std::size_t i, std::size_t j) const
	{
		debug_assert(i < _rows);
		debug_assert(j < _cols);
		return _data[(i * _cols) + j];
	}

	T const& get(std::ptrdiff_t i, std::ptrdiff_t j) const
	{
		auto const [i_, j_] = _wrap(i, j);
		return get(i_, j_);
	}

	template<typename... Args>
	T& get(Args&&... args)
	{
		return const_cast<T&>(std::as_const(*this).get(std::forward<Args>(args)...));
	}


	friend void swap(matrix& first, matrix& second)
	{
		using std::swap;
		swap(first._rows, second._rows);
		swap(first._cols, second._cols);
		swap(first._data, second._data);
	}

private:
	std::size_t _rows;
	std::size_t _cols;
	std::unique_ptr<T[]> _data;

	std::pair<std::size_t, std::size_t> _wrap(std::ptrdiff_t i, std::ptrdiff_t j) const
	{
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

		return {wrap(i, _rows), wrap(j, _cols)};
	}
};


class grid {
public:
	grid(std::size_t rows, std::size_t cols) :
		_curr(rows, cols),
		_next(rows, cols)
	{}

	void rand_init()
	{
		static std::default_random_engine rand_eng(std::chrono::system_clock::now().time_since_epoch().count());
		std::generate_n(_curr.data(), _curr.size(), []() {return rand_eng() & 1u;});
	}

	matrix<bool> const& curr() const
	{
		return _curr;
	}

	matrix<bool> const& next() const
	{
		return _next;
	}

	std::size_t rows() const
	{
		return _curr.rows();
	}

	std::size_t cols() const
	{
		return _curr.cols();
	}

	std::size_t size() const
	{
		return _curr.size();
	}

	bool state(std::size_t i, std::size_t j) const
	{
		return _curr.get(i, j);
	}

	bool update_next(std::size_t i, std::size_t j)
	{
		bool const next = _next_state(i, j);
		_next.get(i, j) = next;
		return next;
	}

	void load_next()
	{
		swap(_curr, _next);
	}

private:
	matrix<bool> _curr;
	matrix<bool> _next;

	
	unsigned _count_neighbours(std::size_t i, std::size_t j) const
	{
		unsigned neighbours = 0;
		std::ptrdiff_t const i_ = i;
		std::ptrdiff_t const j_ = j;
		neighbours += _curr.get(i_ - 1, j_ - 1);
		neighbours += _curr.get(i_ - 1, j_    );
		neighbours += _curr.get(i_ - 1, j_ + 1);
		neighbours += _curr.get(i_    , j_ - 1);
		neighbours += _curr.get(i_    , j_ + 1);
		neighbours += _curr.get(i_ + 1, j_ - 1);
		neighbours += _curr.get(i_ + 1, j_    );
		neighbours += _curr.get(i_ + 1, j_ + 1);
		return neighbours;
	}

	bool _next_state(std::size_t i, std::size_t j) const
	{
		unsigned const neighbours = _count_neighbours(i, j);
		bool const curr_state = _curr.get(i, j);
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


class grid_display {
public:
	grid_display(std::size_t rows, std::size_t cols) :
		_data(rows, cols + 1)
	{
		for (std::size_t i = 0; i < _data.rows(); ++i) {
			_data.get(i, cols) = '\n';
		}
	}

	matrix<char> const& data() const
	{
		return _data;
	}

	std::size_t size() const
	{
		return _data.size();
	}

	void set(std::size_t i, std::size_t j, bool state)
	{
		if (state) {
			_data.get(i, j) = live_cell;
		}
		else {
			_data.get(i, j) = dead_cell;
		}
	}

	void load(grid const& g)
	{
		debug_assert(g.rows() == _data.rows());
		debug_assert(g.cols() == _data.cols() - 1);
		for (std::size_t i = 0; i < _data.rows(); ++i) {
			for (std::size_t j = 0; j < g.cols(); ++j) {
				set(i, j, g.state(i, j));
			}
		}
	}

	void to_console(HANDLE console_handle)
	{
		SetConsoleCursorPosition(console_handle, COORD{0, 0});
		DWORD written;
		WriteConsoleA(console_handle, _data.data(), _data.size(), &written, NULL);
	}

private:
	static constexpr char live_cell = 'x';
	static constexpr char dead_cell = ' ';

	matrix<char> _data;
};


struct barrier {
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


void update_thread(bool& exit, barrier& b, grid& g, std::size_t row_offset, std::size_t rows)
{
	while (!exit) {
		b.wait();
		if (exit) {
			break;
		}
		for (std::size_t i = row_offset; i < row_offset + rows; ++i) {
			for (std::size_t j = 0; j < g.cols(); ++j) {
				g.update_next(i, j);
			}
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
#ifdef BENCHMARK
	constexpr std::size_t rows = BENCHMARK_ROWS;
	constexpr std::size_t cols = BENCHMARK_COLS;
#else
	HANDLE const stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	auto const [rows, cols] = get_console_size(stdout_handle);
	grid_display d(rows, cols);
#endif

	grid g(rows, cols);
	g.rand_init();

	constexpr std::size_t num_threads = 3;

	auto partitions = partition(rows, num_threads);
	std::vector<std::thread> threads;
	threads.reserve(partitions.size());
	barrier b;
	bool exit = false;
	for (auto const& p : partitions) {
		threads.emplace_back(update_thread, std::ref(exit), std::ref(b), std::ref(g), p.first, p.second);
	}

	b.wait_for(threads.size());

#ifdef BENCHMARK
	auto const t1 = std::chrono::high_resolution_clock::now();
	for (std::size_t n = 0; n < BENCHMARK_ITERATIONS; ++n) {
#else
	while (true) {
#endif
		b.release();

#ifndef BENCHMARK
		d.load(g);
		d.to_console(stdout_handle);
#endif

		b.wait_for(threads.size());
		g.load_next();
	}

#ifdef BENCHMARK
	auto const t2 = std::chrono::high_resolution_clock::now();
	auto const time_total = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	auto const iter_per_sec = BENCHMARK_ITERATIONS / (time_total / 1000000.0);
	std::cout << iter_per_sec << " iterations per second" << std::endl;
#endif

	exit = true;
	b.release();
	
	for (auto& t : threads) {
		t.join();
	}
}
