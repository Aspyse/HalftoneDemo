#pragma once

#include "input_system.h"
#include "camera.h"
#include "model.h"
#include "color_shader.h"
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
	bool Render();

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
	InputSystem* m_inputHandle = nullptr;
	CameraClass* m_camera = nullptr;
	ModelClass* m_model = nullptr;
	ColorShader* m_colorShader = nullptr;

	bool m_isSwapChainOccluded = false;

	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;

	ID3D11Texture2D* m_depthStencilBuffer = nullptr;
	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	ID3D11DepthStencilView* m_depthStencilView = nullptr;
	ID3D11RasterizerState* m_rasterState = nullptr;

	XMMATRIX m_projectionMatrix = {};
	XMMATRIX m_worldMatrix = {};
	XMMATRIX m_orthoMatrix = {};

	D3D11_VIEWPORT m_viewport = {};

	UINT m_screenWidth = 0, m_screenHeight = 0;
};