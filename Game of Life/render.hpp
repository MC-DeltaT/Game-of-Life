#pragma once

#include "grid.hpp"

#define NOMINMAX
#include <array>
#include <atlbase.h>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <d2d1.h>
#include <functional>
#include <thread>
#include <vector>
#include <Windows.h>


template<std::size_t Rows, std::size_t Cols>
class window_renderer {
public:
	~window_renderer();
	window_renderer(game_grid<Rows, Cols> const& grid, std::function<void(void)> close_callback);

	void render(std::size_t begin_idx, std::size_t end_idx);
	void draw() const;

private:
	// B8G8R8A8 pixel format.
	static inline constexpr std::array<std::uint8_t, 4> _live_pixel = {0x00, 0xFF, 0x00, 0x00};
	static inline constexpr std::array<std::uint8_t, 4> _dead_pixel = {0x00, 0x00, 0x00, 0x00};

	// Signals for the window to actually be destroyed and the window thread to exit.
	static inline constexpr UINT _wm_definite_close = WM_USER;

	game_grid<Rows, Cols> const* _grid;
	std::vector<std::uint32_t> _pixels;
	HWND _window;
	CComPtr<ID2D1HwndRenderTarget> _render_target;
	CComPtr<ID2D1Bitmap> _bitmap;
	std::thread _window_thread;
	std::atomic_bool _window_ready;
	std::function<void(void)> _close_callback;

	void _window_func();
	static LRESULT CALLBACK _window_proc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
};

template<std::size_t Rows, std::size_t Cols>
class console_renderer {
public:
	console_renderer(game_grid<Rows, Cols> const& grid, HANDLE console_handle);

	void render(std::size_t begin_idx, std::size_t end_idx);
	void draw() const;

private:
	static inline constexpr char live_cell = 'x';
	static inline constexpr char dead_cell = ' ';
	static inline constexpr std::size_t _buf_size = Rows * (Cols + 1);

	game_grid<Rows, Cols> const* _grid;
	std::vector<char> _data;
	HANDLE _console_handle;

	void _render(std::size_t grid_idx, std::size_t buf_idx);
};

class null_renderer {
public:
	template<typename... Args>
	void render(Args&&...) const {}
	template<typename... Args>
	void draw(Args&&...) const {}
};


#include "render.tpp"
