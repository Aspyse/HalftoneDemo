#pragma once

#include "input_system.h"
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

class RenderSystem
{
public:
	RenderSystem();
	RenderSystem(const RenderSystem&);
	~RenderSystem();

	bool Initialize(HWND, WNDCLASSEXW, InputSystem*);
	void Shutdown();
	bool Frame();



private:
	void BeginScene();
	void EndScene();

	bool CreateDeviceD3D(HWND);
	void CleanupDeviceD3D();
	bool CreateRenderTarget();
	void CleanupRenderTarget();

	bool CreateDepthStencilState();
	void CleanupDepthStencilState();
	bool CreateRasterizerState();
	void CleanupRasterizerState();

	bool CreateDepthBuffer();
	void CleanupDepthBuffer();

	void InitializeViewport();
	void InitializeMatrices();

	void SetBackBufferRenderTarget();
	void ResetViewport();

private:
	InputSystem* input_handle = nullptr;

	bool SwapChainOccluded = false;

	IDXGISwapChain* pSwapChain = nullptr;
	ID3D11Device* pd3dDevice = nullptr;
	ID3D11DeviceContext* pd3dDeviceContext = nullptr;
	ID3D11RenderTargetView* mainRenderTargetView = nullptr;

	ID3D11Texture2D* depthStencilBuffer = nullptr;
	ID3D11DepthStencilState* depthStencilState = nullptr;
	ID3D11DepthStencilView* depthStencilView = nullptr;
	ID3D11RasterizerState* rasterState = nullptr;

	XMMATRIX projectionMatrix = {};
	XMMATRIX worldMatrix = {};
	XMMATRIX orthoMatrix = {};

	D3D11_VIEWPORT viewport = {};

	UINT screenWidth = 0, screenHeight = 0;
};