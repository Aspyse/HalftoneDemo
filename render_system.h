#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

class RenderSystem
{
public:
	RenderSystem();
	RenderSystem(const RenderSystem&);
	~RenderSystem();

	bool Initialize(HWND, WNDCLASSEXW);
	void Shutdown();
	bool Frame();

	void BeginScene();
	void EndScene();

private:
	bool CreateDeviceD3D(HWND);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();

private:
	ID3D11Device* pd3dDevice = nullptr;
	ID3D11DeviceContext* pd3dDeviceContext = nullptr;
	IDXGISwapChain* pSwapChain = nullptr;
	ID3D11RenderTargetView* mainRenderTargetView = nullptr;
	bool SwapChainOccluded = false;

	UINT resizeWidth = 0, resizeHeight = 0;
};