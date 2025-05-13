#pragma once

#include <windows.h>
#include "imgui.h"
#include "imgui_impl_win32.h"

class InputSystem {
public:
    InputSystem();
    InputSystem(const InputSystem&);
    ~InputSystem();

    void Initialize();
    bool IsKeyDown(UINT);
    UINT GetResizeWidth();
    UINT GetResizeHeight();

    LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
    void KeyDown(UINT);
    void KeyUp(UINT);

private:
    bool keys[256] = { false };
    UINT resizeWidth = 0, resizeHeight = 0;
};