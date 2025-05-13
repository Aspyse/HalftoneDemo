#include "input_system.h"
#include "imgui_impl_win32.h"

InputSystem::InputSystem() {}
InputSystem::InputSystem(const InputSystem& other) {}
InputSystem::~InputSystem() {}

void InputSystem::Initialize() { }

bool InputSystem::IsKeyDown(UINT input)
{
	return keys[input];
}

UINT InputSystem::GetResizeWidth()
{
	return resizeWidth;
}

UINT InputSystem::GetResizeHeight()
{
	return resizeHeight;
}

void InputSystem::KeyDown(UINT input)
{
	keys[input] = true;
	return;
}

void InputSystem::KeyUp(UINT input)
{
	keys[input] = false;
	return;
}

LRESULT CALLBACK InputSystem::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
	case WM_SIZE:
		if (wparam == SIZE_MINIMIZED)
			return 0;
		resizeWidth = (UINT)LOWORD(lparam);
		resizeHeight = (UINT)HIWORD(lparam);
		return 0;
	case WM_KEYDOWN:
		KeyDown((UINT)wparam);
		return 0;
	case WM_KEYUP:
		KeyUp((UINT)wparam);
		return 0;
	default:
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
}

