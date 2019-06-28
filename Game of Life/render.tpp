#pragma once

#include "grid.hpp"
#include "utility.hpp"

#define NOMINMAX
#include <cstddef>
#include <cstring>
#include <d2d1.h>
#include <exception>
#include <limits>
#include <thread>
#include <Windows.h>


template<std::size_t Rows, std::size_t Cols>
window_renderer<Rows, Cols>::~window_renderer()
{
	DestroyWindow(_window);
	_window_thread.join();
}

template<std::size_t Rows, std::size_t Cols>
window_renderer<Rows, Cols>::window_renderer(game_grid<Rows,Cols> const& grid) :
	_grid(&grid),
	_pixels(Rows * Cols, 0),
	_window_ready(false)
{
	_window_thread = std::thread(&window_renderer::_window_func, this);
	while (!_window_ready) {}

	if (FAILED(CoInitialize(NULL))) {
		throw std::runtime_error("Failed to initialise COM.");
	}

	CComPtr<ID2D1Factory> direct2d_factory = nullptr;
	if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &direct2d_factory))) {
		throw std::runtime_error("Failed to create Direct2D factory.");
	}
	auto target_properties = D2D1::RenderTargetProperties();
	target_properties.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
	auto hwnd_target_properties = D2D1::HwndRenderTargetProperties(_window, D2D1::SizeU(Cols, Rows));

	if (FAILED(direct2d_factory->CreateHwndRenderTarget(target_properties, hwnd_target_properties, &_render_target))) {
		throw std::runtime_error("Failed to create Direct2D render target.");
	}
	direct2d_factory.Release();

	FLOAT dpi_x, dpi_y;
	_render_target->GetDpi(&dpi_x, &dpi_y);
	auto bitmap_properties = D2D1::BitmapProperties(_render_target->GetPixelFormat(), dpi_x, dpi_y);

	if (FAILED(_render_target->CreateBitmap(_render_target->GetPixelSize(), bitmap_properties, &_bitmap))) {
		throw std::runtime_error("Failed to create Direct2D bitmap.");
	}

	ShowWindow(_window, SW_SHOW);
}

template<std::size_t Rows, std::size_t Cols>
void window_renderer<Rows, Cols>::render(std::size_t begin_idx, std::size_t end_idx)
{
	for (std::size_t grid_idx = begin_idx; grid_idx < end_idx; ++grid_idx)
	{
		if (_grid->get_curr(grid_idx)) {
			std::memcpy(_pixels.data() + grid_idx, _live_pixel.data(), 4);
		}
		else {
			std::memcpy(_pixels.data() + grid_idx, _dead_pixel.data(), 4);
		}
	}
}

template<std::size_t Rows, std::size_t Cols>
void window_renderer<Rows, Cols>::draw() const
{
	auto rect = D2D1::RectU(0, 0, Cols, Rows);
	_bitmap->CopyFromMemory(&rect, _pixels.data(), Cols * 4);

	_render_target->BeginDraw();
	_render_target->DrawBitmap(_bitmap);
	if(FAILED(_render_target->EndDraw())) {
		throw std::runtime_error("Failed to draw bitmap.");
	}
}

template<std::size_t Rows, std::size_t Cols>
void window_renderer<Rows, Cols>::_window_func()
{
	LPCTSTR window_class_name = TEXT("Game of Life main");
	LPCTSTR window_title = TEXT("Game of Life");

	WNDCLASS window_class;
	window_class.lpszClassName = window_class_name;
	window_class.hInstance = GetModuleHandle(NULL);
	window_class.lpfnWndProc = &window_renderer::_window_proc;
	window_class.style = 0;
	window_class.hbrBackground = NULL;
	window_class.lpszMenuName = NULL;
	window_class.hCursor = LoadCursor(NULL, IDI_APPLICATION);
	window_class.hIcon = NULL;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;

	if (0 == RegisterClass(&window_class)) {
		throw std::runtime_error("Failed to register window class.");
	}

	DWORD window_style = WS_CAPTION | WS_BORDER | WS_SYSMENU | WS_MINIMIZEBOX;
	RECT window_rect{};
	window_rect.bottom = Rows;
	window_rect.right = Cols;
	if (!AdjustWindowRect(&window_rect, window_style, FALSE)) {
		throw std::runtime_error("Failed to adjust window size.");
	}
	int window_width = window_rect.right - window_rect.left;
	int window_height = window_rect.bottom - window_rect.top;

	_window = CreateWindow(window_class_name, window_title, window_style,
		CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height, NULL, NULL, GetModuleHandle(NULL), NULL);
	if (_window == NULL) {
		throw std::runtime_error("Failed to create window.");
	}

	_window_ready = true;

	MSG msg{};
	while (GetMessage(&msg, _window, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

template<std::size_t Rows, std::size_t Cols>
LRESULT window_renderer<Rows, Cols>::_window_proc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(window, msg, wparam, lparam);
	}
	return 0;
}


template<std::size_t Rows, std::size_t Cols>
console_renderer<Rows, Cols>::console_renderer(game_grid<Rows, Cols> const& grid, HANDLE console_handle) :
	_grid(&grid),
	_data(_buf_size),
	_console_handle(console_handle)
{
	for (std::size_t i = Cols; i < _buf_size; i += Cols + 1) {
		_data[i] = '\n';
	}
}

template<std::size_t Rows, std::size_t Cols>
void console_renderer<Rows, Cols>::render(std::size_t begin_idx, std::size_t end_idx)
{
	debug_assert(end_idx >= begin_idx);

	for (std::size_t grid_idx = begin_idx; grid_idx < end_idx; ++grid_idx) {
		std::size_t buf_idx = grid_idx + (grid_idx / Cols);
		_render(grid_idx, buf_idx);
	}
}

template<std::size_t Rows, std::size_t Cols>
void console_renderer<Rows, Cols>::draw() const
{
	SetConsoleCursorPosition(_console_handle, COORD{0, 0});
	DWORD written;
	static_assert(_buf_size < std::numeric_limits<DWORD>::max());
	debug_assert(WriteConsoleA(_console_handle, _data.data(), static_cast<DWORD>(_buf_size), &written, NULL));
	debug_assert(written == _buf_size);
}

template<std::size_t Rows, std::size_t Cols>
void console_renderer<Rows, Cols>::_render(std::size_t grid_idx, std::size_t buf_idx)
{
	bool const state = _grid->get_curr(grid_idx);
	auto& c = _data[buf_idx];
	if (state) {
		c = live_cell;
	}
	else {
		c = dead_cell;
	}
}
