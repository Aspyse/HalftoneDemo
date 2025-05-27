#include "lighting_pass.h"
#include "halftone_pass.h"
#include "sobel_pass.h"
#include "blend_pass.h"

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

	m_screenWidth = (float) rect.right - rect.left;
	m_screenHeight = (float) rect.bottom - rect.top;

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

	/* SAMPLERS */
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

	// Custom samplers
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



	/* RENDER TARGETS */
	// Stored in vector for management (TODO)
	auto lightingOut = std::make_unique<RenderTarget>();
	lightingOut->Initialize(m_device, m_screenWidth, m_screenHeight);
	m_targets.push_back(std::move(lightingOut)); // target 0

	auto depthIn = std::make_unique<RenderTarget>();
	depthIn->Initialize(m_device, m_screenWidth, m_screenHeight);
	m_targets.push_back(std::move(depthIn)); // target 1

	auto sobelOut = std::make_unique<RenderTarget>();
	sobelOut->Initialize(m_device, m_screenWidth, m_screenHeight);
	m_targets.push_back(std::move(sobelOut)); // target 2

	auto blendIn = std::make_unique<RenderTarget>();
	ID3D11ShaderResourceView* empty[2] = { nullptr, nullptr };
	blendIn->SetResource(empty, 2);
	m_targets.push_back(std::move(blendIn)); // target 3



	/* RENDER PASSES */
	m_geometryPass = new GeometryPass;
	m_geometryPass->Initialize(m_device, m_screenWidth, m_screenHeight);

	auto lightingPass = std::make_unique<LightingPass>();
	lightingPass->Initialize(m_device, L"Shaders/flat.ps");
	lightingPass->Begin = [this, shadowSampler]()
	{
		this->m_deviceContext->PSSetSamplers(1, 1, &shadowSampler);
	};
	m_passes.push_back(std::move(lightingPass)); // pass 0


	auto sobelPass = std::make_unique<SobelPass>();
	sobelPass->Initialize(m_device, L"Shaders/sobel.ps");
	m_passes.push_back(std::move(sobelPass)); // pass 1


	auto blendPass = std::make_unique<BlendPass>();
	blendPass->Initialize(m_device, L"Shaders/blend.ps");
	m_passes.push_back(std::move(blendPass)); // pass 2

	AssignTargets();

	return true;
}

bool RenderSystem::AssignTargets()
{
	m_passes[0]->AssignShaderResource(m_gBuffer, 4);
	m_passes[0]->AssignRenderTarget(m_targets[0]->GetTarget(), 1, nullptr);

	m_passes[1]->AssignShaderResource(m_targets[1]->GetResource(), m_targets[1]->GetNumViews());
	m_passes[1]->AssignRenderTarget(m_targets[2]->GetTarget(), 1, nullptr);

	m_passes[2]->AssignShaderResource(m_targets[3]->GetResource(), m_targets[3]->GetNumViews());
	m_passes[2]->AssignRenderTarget(m_renderTargetView, 1, m_depthStencilView);

	return true;
}

bool RenderSystem::Render(RenderParameters& rParams, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, vector<std::unique_ptr<ModelClass>>& models)
{
	// Handle minimize or screen lock
	if (m_isSwapChainOccluded && m_swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
	{
		::Sleep(10);
		return false;
	}
	m_isSwapChainOccluded = false;
	
	/* GEOMETRY PASS */
	XMFLOAT3 lightDirectionF = XMFLOAT3(rParams.lightDirection[0], rParams.lightDirection[1], rParams.lightDirection[2]);
	XMVECTOR lightDirectionVec = XMVector3Normalize(XMLoadFloat3(&lightDirectionF));
	XMFLOAT3 albedoColor = XMFLOAT3(rParams.albedoColor[0], rParams.albedoColor[1], rParams.albedoColor[2]);

	for (const auto& model : models)
	{
		model->Render(m_deviceContext);
		XMMATRIX worldMatrix = model->GetWorldMatrix();

		m_geometryPass->RenderShadow(m_deviceContext, model->GetIndexCount(), lightDirectionVec);

		// Reset after viewport is set to shadowmap size
		ResetViewport(m_screenWidth, m_screenHeight);

		m_geometryPass->SetShaderParameters(m_deviceContext, worldMatrix, viewMatrix, projectionMatrix, albedoColor, rParams.roughness);
		m_geometryPass->Render(m_deviceContext, model->GetIndexCount(), rParams.clearColor);
	}
	ReleaseRenderTargets();

	// Fetch G-buffer
	// TODO: try to make gbuffer not global
	m_gBuffer[0] = m_geometryPass->GetGBuffer(0);
	m_gBuffer[1] = m_geometryPass->GetGBuffer(1);
	m_gBuffer[2] = m_geometryPass->GetGBuffer(2);
	m_gBuffer[3] = m_geometryPass->GetShadowMap();

	m_targets[1]->SetResource(m_gBuffer[2]); // Pick up depth buffer


	ID3D11ShaderResourceView* blendResources[2] = {
		m_targets[0]->GetResource()[0],
		m_targets[2]->GetResource()[0]
	};

	m_targets[3]->SetResource(blendResources, 2);


	/* LIGHTING PASS */
	/* CONSTANT BUFFER PARAMETERS*/
	XMFLOAT3 lightColor = XMFLOAT3(4.0f, 4.0f, 4.0f);

	// TODO: IMPORTANT! REFACTOR
	if (auto* lp = dynamic_cast<LightingPass*>(m_passes[0].get()))
		lp->SetShaderParameters(m_deviceContext, projectionMatrix, viewMatrix, m_geometryPass->GetLightViewProj(), lightDirectionVec, lightColor, rParams.clearColor, rParams.ambientStrength, rParams.celThreshold);

	if (auto* sp = dynamic_cast<SobelPass*>(m_passes[1].get()))
		sp->SetShaderParameters(m_deviceContext, m_screenWidth, m_screenHeight, 1, rParams.edgeThreshold, rParams.inkColor);

	//if (auto* hp = dynamic_cast<HalftonePass*>(m_passes[1].get()))
		//hp->SetShaderParameters(m_deviceContext, rParams.halftoneDotSize, m_screenWidth, m_screenHeight);



	/* FORWARD RENDER */
	for (auto& pass : m_passes)
	{
		m_deviceContext->PSSetSamplers(0, 1, &m_sampler);
		pass->Render(m_deviceContext, rParams.clearColor);
		ReleaseRenderTargets();
	}



	// Render GUI
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Present
	EndScene();
	return true;
}

void RenderSystem::Resize(UINT width, UINT height)
{
	m_screenWidth = width;
	m_screenHeight = height;
	
	CleanupRenderTarget();
	CleanupDepthBuffer();
	HRESULT hr = m_swapChain->ResizeBuffers(0, m_screenWidth, m_screenHeight, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr))
		return;
	CreateRenderTarget();
	CreateDepthBuffer();

	/* REINITIALIZE + REASSIGN */
	// TODO: IMPORTANT! AUTOMATE
	for (auto& target : m_targets)
	{
		if (target->GetNumViews() == 1) // TODO: fix this nasty workaround
			target->Initialize(m_device, m_screenWidth, m_screenHeight);
	}
	m_geometryPass->InitializeGBuffer(m_device, m_screenWidth, m_screenHeight);

	AssignTargets();

	ResetViewport(m_screenWidth, m_screenHeight);
}


void RenderSystem::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	if (m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = nullptr;
	}

	CleanupDepthBuffer();
	CleanupRenderTarget();
	CleanupDeviceD3D();
}

ID3D11Device* RenderSystem::GetDevice() // For model
{
	return m_device;
}



/* HELPER FUNCTIONS */
void RenderSystem::EndScene()
{
	// Present
	HRESULT hr = m_swapChain->Present(0, 0); // Without vsync
	m_isSwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
}

void RenderSystem::ReleaseRenderTargets()
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
	ID3D11RasterizerState* rasterState = nullptr;
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

	HRESULT result = m_device->CreateRasterizerState(&rd, &rasterState);
	if (FAILED(result))
		return false;

	m_deviceContext->RSSetState(rasterState);

	rasterState->Release();
	rasterState = nullptr;

	return true;
}

void RenderSystem::ResetViewport(float width, float height)
{
	m_viewport.Width = width;
	m_viewport.Height = height;
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