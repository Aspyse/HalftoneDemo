#pragma once

#include "render_parameters.h"
#include "input_system.h"
#include "camera.h"
#include "model.h"
#include "render_target.h"

#include "effect.h"
#include "geometry_pass.h"
#include "canny_pass.h" // temp
#include "out_pass.h"
#include "render_pass.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>

using DirectX::XMFLOAT3;
using DirectX::XMMATRIX;
using std::vector;

class RenderSystem
{
	friend class GuiSystem;

public:
	RenderSystem();
	RenderSystem(const RenderSystem&);
	~RenderSystem();

	bool Initialize(HWND, WNDCLASSEXW);
	void Shutdown();
	bool Render(RenderParameters&, XMMATRIX, XMMATRIX, vector<std::unique_ptr<ModelClass>>&);

	void Resize(UINT, UINT);

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetContext();

private:
	void EndScene();

	bool InitializeDeviceD3D(HWND);
	void CleanupDeviceD3D();
	bool CreateRenderTarget();
	void CleanupRenderTarget();

	bool InitializeDepthStencilState();
	bool InitializeRaster();

	bool CreateDepthBuffer();
	void CleanupDepthBuffer();

	void SetBackBufferRenderTarget();
	void ResetViewport(float, float);

private:
	std::unique_ptr<Effect> m_effect;

	std::vector<ComPtr<ID3D11ShaderResourceView>> m_gBuffer;
	GeometryPass* m_geometryPass = nullptr;
	OutPass* m_outPass = nullptr;

	vector<std::unique_ptr<ModelClass>> m_models;

	bool m_isSwapChainOccluded = false;

	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;

	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	ID3D11DepthStencilView* m_depthStencilView = nullptr;

	ID3D11SamplerState* m_sampler = nullptr;

	D3D11_VIEWPORT m_viewport = {};

	UINT m_screenWidth = 0, m_screenHeight = 0;
};