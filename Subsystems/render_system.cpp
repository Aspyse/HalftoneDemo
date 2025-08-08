#include "lighting_pass.h"
#include "halftone_pass.h"
#include "blend_pass.h"

#include "render_system.h"
#include "imgui_impl_dx11.h"
#include <vector>

RenderSystem::RenderSystem() {};
RenderSystem::RenderSystem(const RenderSystem&) {};
RenderSystem::~RenderSystem() {};

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

	m_geometryPass = new GeometryPass;
	m_geometryPass->Initialize(m_device, m_screenWidth, m_screenHeight);

	m_gBuffer = {
		m_geometryPass->GetGBuffer(0),
		m_geometryPass->GetGBuffer(1),
		m_geometryPass->GetGBuffer(2),
		m_geometryPass->GetShadowMap()
	};

	m_outPass = new OutPass;
	m_outPass->Initialize(m_device, m_screenWidth, m_screenHeight);

	m_effect = std::make_unique<Effect>(m_device, m_deviceContext);
	m_effect->Initialize(m_gBuffer, m_screenWidth, m_screenHeight);

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

	m_geometryPass->ClearGBuffer(m_deviceContext, rParams.clearColor);
	for (const auto& model : models)
	{
		model->SetVertices(m_deviceContext);
		for (UINT i = 0; i < model->GetMaterialCount(); ++i)
		{
			if (!model->IsOpaque(i)) continue;
			
			model->Render(m_device, m_deviceContext, i);
			XMMATRIX worldMatrix = model->GetWorldMatrix();

			m_geometryPass->RenderShadow(m_deviceContext, model->GetIndexCount(i), lightDirectionVec);

			// Reset after viewport is set to shadowmap size
			ResetViewport(m_screenWidth, m_screenHeight);

			m_geometryPass->Update(m_deviceContext, viewMatrix, projectionMatrix, model->GetUseTexture(i, 0), model->GetUseTexture(i, 1), model->GetUseTexture(i, 2));
			m_geometryPass->Render(m_deviceContext, model->GetIndexCount(i));
		}
	}
	ID3D11RenderTargetView* nullRTV = nullptr;
	m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

	XMFLOAT3 clearColor = XMFLOAT3(rParams.clearColor[0]*rParams.ambientStrength, rParams.clearColor[1]* rParams.ambientStrength, rParams.clearColor[2]* rParams.ambientStrength);
	m_effect->Update(viewMatrix, projectionMatrix, m_geometryPass->GetLightViewProj(), lightDirectionVec, clearColor);
	m_effect->Render(m_sampler);

	auto outTarget = m_outPass->GetInputs()[0];
	auto it = m_effect->GetTargets().find(outTarget);
	if (it != m_effect->GetTargets().end())
	{
		m_deviceContext->PSSetShaderResources(0, 1, m_effect->GetTargets().at(outTarget)->GetResourceView());
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
		m_outPass->Render(m_device, m_deviceContext, rParams.clearColor);

		ID3D11RenderTargetView* nullRTV = nullptr;
		m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
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

	m_geometryPass->InitializeGBuffer(m_device, m_screenWidth, m_screenHeight);
	m_gBuffer = {
		m_geometryPass->GetGBuffer(0),
		m_geometryPass->GetGBuffer(1),
		m_geometryPass->GetGBuffer(2),
		m_geometryPass->GetShadowMap()
	};
	//m_effect->Resize(width, height);
	m_effect->Initialize(m_gBuffer, width, height);
	
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

ID3D11DeviceContext* RenderSystem::GetContext() // For model
{
	return m_deviceContext;
}



/* HELPER FUNCTIONS */
void RenderSystem::EndScene()
{
	// Present
	HRESULT hr = m_swapChain->Present(0, 0); // Without vsync
	m_isSwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
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

	UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
	
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