#include "Window.h"

Window* window = nullptr;

Window::Window()
	: m_hwnd(nullptr)
	, m_is_run(false)
	, msg{}
{
}

LRESULT CALLBACK Wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		window->setHWND(hwnd); // FIX: assign HWND before onCreate() fires,
		window->onCreate();    // because CreateWindowEx hasn't returned yet
		break;                 // so m_hwnd would still be nullptr otherwise
	}

	case WM_DESTROY:
	{
		window->onDestroy();
		::PostQuitMessage(0);
		break;
	}

	default:
		return ::DefWindowProc(hwnd, msg, wparam, lparam);
	}

	return NULL;
}

bool Window::init()
{
	if (!window)
		window = this;

	WNDCLASSEX wc;
	wc.cbClsExtra = NULL;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbWndExtra = NULL;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = NULL;
	wc.lpszClassName = L"MyWindowClass";
	wc.lpszMenuName = L"";
	wc.style = NULL;
	wc.lpfnWndProc = Wndproc;

	if (!RegisterClassEx(&wc))
		return false;

	m_hwnd = ::CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		L"MyWindowClass",
		L"DirectX Application",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		1024, 768,
		NULL, NULL, NULL, NULL);

	if (!m_hwnd)
		return false;

	::ShowWindow(m_hwnd, SW_SHOW);
	::UpdateWindow(m_hwnd);

	m_is_run = true;

	return true;
}

bool Window::broadcast()
{
	this->onUpdate();

	while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	Sleep(1);

	return true;
}

bool Window::release()
{
	if (!::DestroyWindow(m_hwnd))
		return false;

	return true;
}

bool Window::isRun()
{
	return m_is_run;
}

RECT Window::getClientWindowRect() const
{
	RECT rc;
	::GetClientRect(m_hwnd, &rc);
	return rc;
}

void Window::setHWND(HWND hwnd)
{
	m_hwnd = hwnd;
}

void Window::onDestroy()
{
	m_is_run = false;
}

Window::~Window()
{
}