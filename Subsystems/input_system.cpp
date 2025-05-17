#include "input_system.h"
#include "imgui_impl_win32.h"

InputSystem::InputSystem() {}
InputSystem::InputSystem(const InputSystem& other) {}
InputSystem::~InputSystem() {}

void InputSystem::Initialize() { }

bool InputSystem::IsKeyDown(UINT input)
{
	return m_keys[input];
}

int InputSystem::GetScrollDelta()
{
	int delta = m_scrollDelta;
	m_scrollDelta = 0; // TODO: test reset
	return delta;
}

bool InputSystem::IsMiddleMouseDown()
{
	return m_isMiddleMouseDown;
}

POINT InputSystem::GetMousePos()
{
	return m_lastMousePos;
}

UINT InputSystem::GetResizeWidth()
{
	return m_resizeWidth;
}

UINT InputSystem::GetResizeHeight()
{
	return m_resizeHeight;
}

void InputSystem::KeyDown(UINT input)
{
	m_keys[input] = true;
	return;
}

void InputSystem::KeyUp(UINT input)
{
	m_keys[input] = false;
	return;
}

LRESULT CALLBACK InputSystem::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
	case WM_SYSCOMMAND:
		if ((wparam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		if (wparam == SIZE_MINIMIZED)
			return 0;
		m_resizeWidth = (UINT)LOWORD(lparam);
		m_resizeHeight = (UINT)HIWORD(lparam);
		return 0;

	case WM_KEYDOWN:
		KeyDown((UINT)wparam);
		return 0;
	case WM_KEYUP:
		KeyUp((UINT)wparam);
		return 0;

	case WM_MOUSEWHEEL:
		m_scrollDelta += GET_WHEEL_DELTA_WPARAM(wparam);
		return 0;
	case WM_MBUTTONDOWN:
		m_isMiddleMouseDown = true;
		SetCapture(hwnd);
		return 0;
	case WM_MBUTTONUP:
		m_isMiddleMouseDown = false;
		ReleaseCapture();
		return 0;

	case WM_MOUSEMOVE:
	{
		m_lastMousePos.x = (int)(short)LOWORD(lparam);
		m_lastMousePos.y = (int)(short)HIWORD(lparam);
		return 0;
	}
	}

	return ::DefWindowProcW(hwnd, umsg, wparam, lparam);
}

