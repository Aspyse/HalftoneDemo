#include "lighting_pass.h"
#include "halftone_pass.h"

#include "render_system.h"
#include "imgui_impl_dx11.h"
#include <vector>

RenderSystem::RenderSystem() {};
RenderSystem::RenderSystem(const RenderSystem&) {};
RenderSystem::~RenderSystem() {};
using namespace std;

bool RenderSystem::Initialize(HWND hwnd, WNDCLASSEXW wc)
{
	RECT rect;
	GetClientRect(hwnd, &rect);

	m_screenWidth = rect.right - rect.left;
	m_screenHeight = rect.bottom - rect.top;

	// Initialize Direct3D
	if (!InitializeDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return false;
	}

	ImGui_ImplDX11_Init(m_device, m_deviceContext);

	CreateRenderTarget();
	InitializeDepthStencilState();
	CreateDepthBuffer();

	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	InitializeRaster();
	ResetViewport(m_screenWidth, m_screenHeight);

	// Sampler init
	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.MipLODBias = 0.0f;
	sd.MaxAnisotropy = 1;
	sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sd.BorderColor[0] = 0;
	sd.BorderColor[1] = 0;
	sd.BorderColor[2] = 0;
	sd.BorderColor[3] = 0;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT result = m_device->CreateSamplerState(&sd, &m_sampler);
	if (FAILED(result))
		return false;

	m_deviceContext->PSSetSamplers(0, 1, &m_sampler);
	
	m_geometryPass = new GeometryPass;
	m_geometryPass->Initialize(m_device, m_screenWidth, m_screenHeight);

	// Init render targets, stored in vector for management (TODO)
	auto lightingOut = std::make_unique<RenderTarget>();
	lightingOut->Initialize(m_device, m_screenWidth, m_screenHeight);
	m_targets.push_back(std::move(lightingOut));

	auto halftoneOut = std::make_unique<RenderTarget>();
	halftoneOut->SetTarget(m_renderTargetView);
	m_targets.push_back(std::move(halftoneOut));

	// Shadow comparison sampler
	ID3D11SamplerState* shadowSampler;
	D3D11_SAMPLER_DESC shadowDesc;
	ZeroMemory(&shadowDesc, sizeof(shadowDesc));
	shadowDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	shadowDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	shadowDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	shadowDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	shadowDesc.MipLODBias = 0.0f;
	shadowDesc.MaxAnisotropy = 1;
	shadowDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	shadowDesc.BorderColor[0] = 1.0f;
	shadowDesc.BorderColor[1] = 1.0f;
	shadowDesc.BorderColor[2] = 1.0f;
	shadowDesc.BorderColor[3] = 1.0f;
	shadowDesc.MinLOD = 0;
	shadowDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = m_device->CreateSamplerState(&shadowDesc, &shadowSampler);
	if (FAILED(result))
		return false;

	// Render passes
	auto lightingPass = std::make_unique<LightingPass>();
	lightingPass->Initialize(m_device, L"Shaders/flat.ps");
	lightingPass->Begin = [this, shadowSampler]()
	{
		this->m_deviceContext->PSSetSamplers(1, 1, &shadowSampler);
	};
	lightingPass->AssignShaderResource(m_gBuffer, 4);
	//lightingPass->AssignRenderTarget(m_renderTargetView, 1, m_depthStencilView);
	lightingPass->AssignRenderTarget(m_targets[0]->GetTarget(), 1, nullptr);
	m_passes.push_back(std::move(lightingPass));


	auto halftonePass = std::make_unique<HalftonePass>();
	halftonePass->Initialize(m_device, L"Shaders/halftone.ps");
	halftonePass->AssignShaderResource(m_targets[0]->GetResource(), 1);
	halftonePass->AssignRenderTarget(m_targets[1]->GetTarget(), 1, m_depthStencilView);
	m_passes.push_back(std::move(halftonePass));


	return true;
}

bool RenderSystem::Render(RenderParameters& rParams, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, vector<std::unique_ptr<ModelClass>>& models)
{
	XMFLOAT3 lightDirectionF = XMFLOAT3(rParams.lightDirection[0], rParams.lightDirection[1], rParams.lightDirection[2]);
	XMVECTOR lightDirectionVec = XMVector3Normalize(XMLoadFloat3(&lightDirectionF));
	XMFLOAT3 albedoColor = XMFLOAT3(rParams.albedoColor[0], rParams.albedoColor[1], rParams.albedoColor[2]);

	for (const auto& model : models)
	{
		model->Render(m_deviceContext);
		XMMATRIX worldMatrix = model->GetWorldMatrix();

		// GEOMETRY PASS
		m_geometryPass->RenderShadow(m_deviceContext, model->GetIndexCount(), lightDirectionVec);

		// Reset after viewport is set to shadowmap size
		ResetViewport(m_screenWidth, m_screenHeight);

		m_geometryPass->SetShaderParameters(m_deviceContext, worldMatrix, viewMatrix, projectionMatrix, albedoColor, rParams.roughness);
		m_geometryPass->Render(m_deviceContext, model->GetIndexCount(), rParams.clearColor);
	}
	ClearRenderTargets();

	m_gBuffer[0] = m_geometryPass->GetGBuffer(0);
	m_gBuffer[1] = m_geometryPass->GetGBuffer(1);
	m_gBuffer[2] = m_geometryPass->GetGBuffer(2);
	m_gBuffer[3] = m_geometryPass->GetShadowMap();


	XMFLOAT3 lightColor = XMFLOAT3(4.0f, 4.0f, 4.0f);
	float celThreshold = 0.0f;

	// TODO: URGENT REFACTOR
	if (auto* lp = dynamic_cast<LightingPass*>(m_passes[0].get()))
		lp->SetShaderParameters(m_deviceContext, projectionMatrix, viewMatrix, m_geometryPass->GetLightViewProj(), lightDirectionVec, lightColor, rParams.clearColor, rParams.ambientStrength, celThreshold);
	if (auto* hp = dynamic_cast<HalftonePass*>(m_passes[1].get()))
		hp->SetShaderParameters(m_deviceContext, rParams.halftoneDotSize);

	for (auto& pass : m_passes)
	{
		pass->Render(m_deviceContext, rParams.clearColor);
	}

	/* OLD PIPELINE
	// POST-PROCESS (HALFTONE)
	ID3D11ShaderResourceView* srv = m_halftoneRT->GetResource();
	m_deviceContext->PSSetShaderResources(0, 1, &srv);

	// Clear the buffers to begin the scene
	BeginScene();
	ResetViewport(m_screenWidth, m_screenHeight);

	m_halftoneShader->SetShaderParameters(m_deviceContext, m_halftoneDotSize);
	m_halftoneShader->Render(m_deviceContext);

	END OF OLD PIPELINE*/

	// Render GUI
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // TODO: consider decoupling with GUI

	// Present the rendered scene to the screen
	EndScene();
	return true; // TEMP
}

void RenderSystem::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	
	if (m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = nullptr;
	}
	if (m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = nullptr;
	}

	CleanupDepthBuffer();
	CleanupRenderTarget();
	CleanupDeviceD3D();
}

void RenderSystem::Resize(UINT width, UINT height)
{
	m_screenWidth = width;
	m_screenHeight = height;
	
	CleanupRenderTarget();
	CleanupDepthBuffer();
	m_swapChain->ResizeBuffers(0, m_screenWidth, m_screenHeight, DXGI_FORMAT_UNKNOWN, 0);

	CreateRenderTarget();
	CreateDepthBuffer();

	ResetViewport(m_screenWidth, m_screenHeight);

	// TODO: resize list of rendertargets

	for (auto& target : m_targets)
	{
		target->Initialize(m_device, m_screenWidth, m_screenHeight);
	}

	m_geometryPass->InitializeGBuffer(m_device, m_screenWidth, m_screenHeight);
	//m_passes[1]->AssignRenderTarget(m_renderTargetView, 1, m_depthStencilView);
}

ID3D11Device* RenderSystem::GetDevice()
{
	return m_device;
}



// HELPER FUNCTIONS
void RenderSystem::BeginScene(float* clearColor)
{
	// Handle minimize or screen lock
	if (m_isSwapChainOccluded && m_swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
	{
		::Sleep(10);
		return;
	}
	m_isSwapChainOccluded = false;

	m_deviceContext->ClearRenderTargetView(m_renderTargetView, clearColor);
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void RenderSystem::EndScene()
{
	// Present
	HRESULT hr = m_swapChain->Present(0, 0); // Without vsync
	m_isSwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
}

void RenderSystem::ClearRenderTargets()
{
	ID3D11RenderTargetView* nullRTV = nullptr;
	m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
}

bool RenderSystem::InitializeDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	sd.Flags = 0;

	UINT createDeviceFlags = 0;
	
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, &featureLevel, &m_deviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, &featureLevel, &m_deviceContext);
	if (res != S_OK)
		return false;

	return true;
}

void RenderSystem::CleanupDeviceD3D()
{
	if (m_deviceContext)
	{
		m_deviceContext->Release();
		m_deviceContext = nullptr;
	}
	if (m_device)
	{
		m_device->Release();
		m_device = nullptr;
	}
	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = nullptr;
	}
}

bool RenderSystem::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	HRESULT result = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (FAILED(result))
		return false;

	result = m_device->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTargetView);
	if (FAILED(result))
		return false;

	pBackBuffer->Release();
	pBackBuffer = nullptr;

	return true;
}

void RenderSystem::CleanupRenderTarget()
{
	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = nullptr;
	}
}

bool RenderSystem::InitializeDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC dsd;
	ZeroMemory(&dsd, sizeof(dsd));
	dsd.DepthEnable = true;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;

	dsd.StencilEnable = true;
	dsd.StencilReadMask = 0xFF;
	dsd.StencilWriteMask = 0xFF;

	dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HRESULT result = m_device->CreateDepthStencilState(&dsd, &m_depthStencilState);
	if (FAILED(result))
		return false;

	return true;
}

bool RenderSystem::CreateDepthBuffer()
{
	D3D11_TEXTURE2D_DESC dbd;
	ZeroMemory(&dbd, sizeof(dbd));
	dbd.Width = m_screenWidth;
	dbd.Height = m_screenHeight;
	dbd.MipLevels = 1;
	dbd.ArraySize = 1;
	dbd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dbd.SampleDesc.Count = 1;
	dbd.SampleDesc.Quality = 0;
	dbd.Usage = D3D11_USAGE_DEFAULT;
	dbd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dbd.CPUAccessFlags = 0;
	dbd.MiscFlags = 0;

	ID3D11Texture2D* depthStencilBuffer;
	HRESULT result = m_device->CreateTexture2D(&dbd, nullptr, &depthStencilBuffer);
	if (FAILED(result))
		return false;

	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	// Stencil view descriptioon
	D3D11_DEPTH_STENCIL_VIEW_DESC svd;
	ZeroMemory(&svd, sizeof(svd));
	svd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	svd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	svd.Texture2D.MipSlice = 0;

	result = m_device->CreateDepthStencilView(depthStencilBuffer, &svd, &m_depthStencilView);
	if (FAILED(result))
		return false;

	depthStencilBuffer->Release();
	depthStencilBuffer = nullptr;

	return true;
}

void RenderSystem::CleanupDepthBuffer()
{
	if (m_depthStencilView) {
		m_depthStencilView->Release();
		m_depthStencilView = nullptr;
	}
}

bool RenderSystem::InitializeRaster()
{
	D3D11_RASTERIZER_DESC rd;
	ZeroMemory(&rd, sizeof(rd));
	rd.AntialiasedLineEnable = false;
	rd.CullMode = D3D11_CULL_BACK;
	rd.DepthBias = 0;
	rd.DepthBiasClamp = 0.0f;
	rd.DepthClipEnable = true;
	rd.FillMode = D3D11_FILL_SOLID;
	rd.MultisampleEnable = false;
	rd.ScissorEnable = false;
	rd.SlopeScaledDepthBias = 0.0f;

	HRESULT result = m_device->CreateRasterizerState(&rd, &m_rasterState);
	if (FAILED(result))
		return false;

	m_deviceContext->RSSetState(m_rasterState);

	return true;
}

void RenderSystem::ResetViewport(float width, float height)
{
	m_viewport.Width = static_cast<FLOAT>(width);
	m_viewport.Height = static_cast<FLOAT>(height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	m_deviceContext->RSSetViewports(1, &m_viewport);
}

void RenderSystem::SetBackBufferRenderTarget()
{
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
}