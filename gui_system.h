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
	bool Frame();

private:
	WNDCLASSEXW wc;
	HWND hwnd = nullptr;

	ImGuiIO* io = nullptr;
	RenderSystem* render_system = nullptr;
};

static LRESULT WINAPI WndProc(HWND, UINT, WPARAM, LPARAM);