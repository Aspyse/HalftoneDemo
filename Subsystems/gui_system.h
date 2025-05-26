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

	bool Initialize(HWND);
	void Shutdown();
	bool Frame(RenderParameters&);

private:
	ImGuiIO* m_io = nullptr;
	RenderSystem* m_renderSystem = nullptr;
};

static LRESULT WINAPI WndProc(HWND, UINT, WPARAM, LPARAM);