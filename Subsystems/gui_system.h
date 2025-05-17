#pragma once

#include "input_system.h"
#include "render_system.h"
#include "imgui.h"
#include <d3d11.h>
#include <windows.h>

class GuiSystem
{
public:
	GuiSystem();
	GuiSystem(const GuiSystem&);
	~GuiSystem();

	bool Initialize(InputSystem*);
	void Shutdown();
	bool Frame(RenderSystem*);

	WNDCLASSEXW GetWC();
	HWND GetHWND();

private:
	WNDCLASSEXW m_wc;
	HWND m_hwnd = nullptr;

	ImGuiIO* m_io = nullptr;
	RenderSystem* m_renderSystem = nullptr;
};

static LRESULT WINAPI WndProc(HWND, UINT, WPARAM, LPARAM);