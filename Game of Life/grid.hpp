#pragma once

#include "utility.hpp"

#include <cstddef>
#include <cstdint>


template<std::size_t Rows, std::size_t Cols>
class game_grid {
public:
	static_assert(Rows >= 2 && Cols >= 2);

	static inline constexpr std::size_t rows = Rows;
	static inline constexpr std::size_t cols = Cols;
	static inline constexpr std::size_t size = Rows * Cols;

	game_grid();

	std::uint8_t* curr();
	std::uint8_t const* curr() const;
	std::uint8_t* next();
	std::uint8_t const* next() const;

	bool get_curr(std::size_t idx) const;
	void set_next(std::size_t idx, bool state);

	void load_next();
	void rand_init();

private:
	aligned_array<std::uint8_t, 32> _curr;
	aligned_array<std::uint8_t, 32> _next;
};


#include "grid.tpp"
