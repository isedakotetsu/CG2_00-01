#pragma once
#include <Windows.h>
#include <numbers>


class WinApp
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	void Initialize();
	void Update();
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;
	HWND GetHwnd() const { return hwnd; }
	HINSTANCE GetHInstance() const { return wc.hInstance; }
private:
	HWND hwnd = nullptr;
	WNDCLASS wc{};
};
