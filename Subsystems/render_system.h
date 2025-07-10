#pragma once

#include "render_parameters.h"
#include "input_system.h"
#include "camera.h"
#include "model.h"
#include "render_target.h"

#include "render_graph.h"
#include "geometry_pass.h"
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
	//bool AssignTargets();

	void EndScene();

	bool InitializeDeviceD3D(HWND);
	void CleanupDeviceD3D();
	bool CreateRenderTarget();
	void CleanupRenderTarget();
	//void ReleaseRenderTargets();

	bool InitializeDepthStencilState();
	bool InitializeRaster();

	bool CreateDepthBuffer();
	void CleanupDepthBuffer();

	void SetBackBufferRenderTarget();
	void ResetViewport(float, float);

private:
	std::unique_ptr<RenderGraph> m_material;

	ID3D11ShaderResourceView* m_gBuffer[4] = { nullptr, nullptr, nullptr, nullptr };
	GeometryPass* m_geometryPass = nullptr;

	vector<std::unique_ptr<ModelClass>> m_models;
	vector<std::unique_ptr<RenderTarget>> m_targets;
	vector<std::unique_ptr<RenderPass>> m_passes;

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