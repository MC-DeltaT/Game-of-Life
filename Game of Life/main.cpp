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


struct grid {
	std::size_t rows;
	std::size_t cols;
	std::size_t size;
	std::unique_ptr<bool[]> data1;
	std::unique_ptr<bool[]> data2;
	bool* curr;
	bool* next;

	grid(std::size_t rows, std::size_t cols) :
		rows(rows), cols(cols), size(rows * cols),
		data1(new bool[size]), data2(new bool[size]),
		curr(data1.get()), next(data2.get())
	{}

	void rand_init()
	{
		static std::default_random_engine rand_eng(std::chrono::system_clock::now().time_since_epoch().count());
		std::generate_n(curr, size, []() {return rand_eng() & 1u;});
	}

	std::size_t index_wrap(std::ptrdiff_t i, std::ptrdiff_t j) const
	{
		if (i < 0) {
			i = rows + i;
		}
		else if (i >= rows) {
			i %= rows;
		}

		if (j < 0) {
			j = cols + j;
		}
		else if (j >= cols) {
			j %= cols;
		}

		assert(i >= 0);
		assert(j >= 0);
		return index(i, j);
	}

	std::size_t index(std::size_t i, std::size_t j) const
	{
		assert(i < rows);
		assert(j < cols);
		return (i * cols) + j;
	}

	bool get_curr(std::size_t i, std::size_t j) const
	{
		return curr[index(i, j)];
	}

	bool get_curr_wrap(std::ptrdiff_t i, std::ptrdiff_t j) const
	{
		return curr[index_wrap(i, j)];
	}

	void set_next(std::size_t i, std::size_t j, bool state)
	{
		next[index(i, j)] = state;
	}

	unsigned count_neighbours(std::size_t i_, std::size_t j_) const
	{
		std::ptrdiff_t i = i_;
		std::ptrdiff_t j = j_;
		unsigned neighbours = 0;
		neighbours += get_curr_wrap(i - 1, j - 1);
		neighbours += get_curr_wrap(i - 1, j    );
		neighbours += get_curr_wrap(i - 1, j + 1);
		neighbours += get_curr_wrap(i    , j - 1);
		neighbours += get_curr_wrap(i    , j + 1);
		neighbours += get_curr_wrap(i + 1, j - 1);
		neighbours += get_curr_wrap(i + 1, j    );
		neighbours += get_curr_wrap(i + 1, j + 1);
		return neighbours;
	}

	bool next_state(std::size_t i, std::size_t j) const
	{
		unsigned neighbours = count_neighbours(i, j);
		bool curr_state = get_curr(i, j);
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

	void update(std::size_t i, std::size_t j)
	{
		set_next(i, j, next_state(i, j));
	}

	void update()
	{
		for (std::size_t i = 0; i < rows; ++i) {
			for (std::size_t j = 0; j < cols; ++j) {
				update(i, j);
			}
		}
		std::swap(curr, next);
	}
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
	assert(rows > 0);
	while (!exit) {
		b.wait();
		if (exit) {
			break;
		}
		for (std::size_t i = row_offset; i < row_offset + rows; ++i) {
			for (std::size_t j = 0; j < g.cols; ++j) {
				g.update(i, j);
			}
		}
	}
}


std::vector<std::pair<std::size_t, std::size_t>> partition(std::size_t val, std::size_t partitions)
{
	std::size_t actual_partitions = val < partitions ? val : partitions;
	std::size_t div = val / actual_partitions;
	std::size_t mod = val % actual_partitions;

	std::vector<std::pair<std::size_t, std::size_t>> res;
	res.reserve(actual_partitions);
	std::size_t offset = 0;
	for (std::size_t i = 0; i < actual_partitions; ++i) {
		std::size_t extra = mod > 0 ? 1 : 0;
		res.emplace_back(offset, div + extra);
		offset += div + extra;
		mod -= extra;
	}

#ifdef _DEBUG
	std::size_t total = 0;
	std::size_t next_offset = 0;
	for (auto const& p : res) {
		assert(p.first == next_offset);
		total += p.second;
		next_offset = p.first + p.second;
	}
	assert(next_offset == val);
	assert(total == val);
#endif

	return res;
}


int main()
{
	constexpr char live_cell = 'x';
	constexpr char dead_cell = ' ';
	constexpr char newline = '\n';

	std::ios_base::sync_with_stdio(false);

	HANDLE const stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(stdout_handle, &csbi);

	std::size_t const rows = csbi.srWindow.Bottom - csbi.srWindow.Top;
	std::size_t const cols = csbi.srWindow.Right - csbi.srWindow.Left;

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

	while (true) {
		SetConsoleCursorPosition(stdout_handle, COORD{0, 0});
		for (std::size_t i = 0; i < rows; ++i) {
			for (std::size_t j = 0; j < cols; ++j) {
				if (g.get_curr(i, j)) {
					std::cout.write(&live_cell, 1);
				}
				else {
					std::cout.write(&dead_cell, 1);
				}
			}
			std::cout.write(&newline, 1);
		}

		b.wait_for(threads.size());

		if (std::none_of(g.curr, g.curr + rows * cols, [](auto x) { return x; })) {
			g.rand_init();
		}
		else if (std::equal(g.curr, g.curr + rows * cols, g.next)) {
			g.rand_init();
		}
		else {
			std::swap(g.curr, g.next);
		}

		b.release();
	}

	b.wait_for(threads.size());
	exit = true;
	b.release();
	
	for (auto& t : threads) {
		t.join();
	}
}