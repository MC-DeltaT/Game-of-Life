#pragma once

#include <array>
#include <cstddef>
#include <memory>


template<std::size_t Rows, std::size_t Cols>
class game_grid {
public:
	static_assert(Rows >= 2 && Cols >= 2);

	static inline constexpr std::size_t rows = Rows;
	static inline constexpr std::size_t cols = Cols;
	static inline constexpr std::size_t size = Rows * Cols;
	static inline constexpr std::size_t neighbour_count = 8;
	static inline constexpr std::array<std::ptrdiff_t, neighbour_count> neighbour_offsets{
		-std::ptrdiff_t{Cols} - 1, -std::ptrdiff_t{Cols}, -std::ptrdiff_t{Cols} + 1,
		-1,												  1,
		Cols - 1,				   Cols,				  Cols + 1
	};

	game_grid();

	bool get_curr(std::size_t idx) const;
	void set_next(std::size_t idx, bool state) const;

	void load_next();
	void rand_init();

private:
	std::unique_ptr<bool[]> _curr;
	std::unique_ptr<bool[]> _next;
};


#include "grid.tpp"
