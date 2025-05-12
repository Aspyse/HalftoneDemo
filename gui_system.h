#pragma once

#include "input_system.h"
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

	void Resize(UINT, UINT);
	InputSystem* GetInputHandle();

private:
	bool CreateDeviceD3D(HWND);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();

private:
	InputSystem* input_handle = nullptr;

	ID3D11Device* pd3dDevice = nullptr;
	ID3D11DeviceContext* pd3dDeviceContext = nullptr;
	IDXGISwapChain* pSwapChain = nullptr;
	ID3D11RenderTargetView* mainRenderTargetView = nullptr;
	bool SwapChainOccluded = false;
	UINT resizeWidth = 0, resizeHeight = 0;

	WNDCLASSEXW wc;
	HWND hwnd = nullptr;

	ImGuiIO* io = nullptr;
};

static LRESULT WINAPI WndProc(HWND, UINT, WPARAM, LPARAM);