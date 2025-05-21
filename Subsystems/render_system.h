#pragma once

#include "input_system.h"
#include "camera.h"
#include "model.h"
#include "geometry_pass.h"
#include "lighting_shader.h"
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
	bool Render(InputSystem*);

	// Menu values
	float* LightDirection();
	float* ClearColor();
	float& AmbientStrength();
	float& CelThreshold();
	float& Roughness();

	bool ResetModel(const char*);

private:
	void BeginScene();
	void EndScene();

	bool CreateDeviceD3D(HWND);
	void CleanupDeviceD3D();
	bool CreateRenderTarget();
	void CleanupRenderTarget();

	bool CreateDepthStencilState();
	bool CreateRasterState();

	bool CreateDepthBuffer();
	void CleanupDepthBuffer();

	void InitializeViewport(float, float);
	void InitializeMatrices();

	void SetBackBufferRenderTarget();
	void ResetViewport();

private:
	CameraClass* m_camera = nullptr;
	ModelClass* m_model = nullptr;
	GeometryPass* m_geometryPass = nullptr;
	LightingShader* m_lightingShader = nullptr;

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

	// Menu values
	float m_lightDirection[3] = { 1.0f, -1.0f, 1.0f };
	float m_clearColor[3] = { 0.9f, 1.0f, 1.0f };
	float m_ambientStrength = 0.5f;
	float m_celThreshold = 0.4f;
	float m_roughness = 0.16f;

	const XMMATRIX m_shadowMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(0.5f, 0.5f, 0.5f);
};