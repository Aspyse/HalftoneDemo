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

    int GetScrollDelta();
    bool IsMiddleMouseDown();
    POINT GetMousePos();

    bool IsResizeDirty();
    UINT GetResizeWidth();
    UINT GetResizeHeight();

    LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
    void KeyDown(UINT);
    void KeyUp(UINT);

private:
    bool m_keys[256] = { false };
    bool m_isMiddleMouseDown = false;
    POINT m_mousePos = { 0, 0 };
    bool m_isResizeDirty = false;
    UINT m_resizeWidth = 0, m_resizeHeight = 0;
    int m_scrollDelta = 0;
};