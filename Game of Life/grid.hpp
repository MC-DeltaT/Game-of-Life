#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>


class game_grid {
public:
	game_grid(std::size_t rows, std::size_t cols);

	std::size_t rows() const;
	std::size_t cols() const;

	void rand_init();
	bool curr_state(std::size_t i) const;
	bool update_next(std::size_t i);
	void load_next();

private:
	static constexpr std::array<std::pair<std::ptrdiff_t, std::ptrdiff_t>, 8> _neighbour_offsets{
		std::pair{-1, -1}, std::pair{-1, 0}, std::pair{-1, 1},
		std::pair{0, -1},					 std::pair{0, 1},
		std::pair{1, -1},  std::pair{1, 0},  std::pair{1, 1}
	};

	std::size_t _rows;
	std::size_t _cols;
	std::unique_ptr<bool[]> _curr;
	std::unique_ptr<bool[]> _next;
	std::vector<std::array<std::size_t, _neighbour_offsets.size()>> _neighbour_lut;
	
	unsigned _count_neighbours(std::size_t i) const;

	bool _next_state(std::size_t i) const;
};