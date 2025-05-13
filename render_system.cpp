#include "render_system.h"
#include "camera.h"
#include "model.h"
#include "color_shader.h"
#include "imgui_impl_dx11.h"

RenderSystem::RenderSystem() {};
RenderSystem::RenderSystem(const RenderSystem&) {};
RenderSystem::~RenderSystem() {};

bool RenderSystem::Initialize(HWND hwnd, WNDCLASSEXW wc, InputSystem* inputHandle)
{
	RECT rect;
	GetClientRect(hwnd, &rect);

	screenWidth = rect.right - rect.left;
	screenHeight = rect.bottom - rect.top;
	input_handle = inputHandle;
	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return false;
	}

	ImGui_ImplDX11_Init(pd3dDevice, pd3dDeviceContext);

	CreateRenderTarget();
	CreateDepthStencilState();
	CreateDepthBuffer();

	pd3dDeviceContext->OMSetRenderTargets(1, &mainRenderTargetView, depthStencilView);

	CreateRasterizerState();
	InitializeViewport();
	InitializeMatrices();

	return true;
}

bool RenderSystem::Render()
{
	// Clear the buffers to begin the scene
	BeginScene();

	// Scene

	// Render GUI
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // TODO: consider decoupling with GUI

	// Present the rendered scene to the screen
	EndScene();
	return true; // TEMP
}

void RenderSystem::BeginScene()
{
	// Handle minimize or screen lock
	if (SwapChainOccluded && pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
	{
		::Sleep(10);
		return;
	}
	SwapChainOccluded = false;

	// Handle window resize (we don't resize directly in the WM_SIZE handler)
	screenWidth = input_handle->GetResizeWidth();
	screenHeight = input_handle->GetResizeHeight();
	if (screenWidth != 0 && screenHeight != 0)
	{
		CleanupRenderTarget();
		CleanupDepthBuffer();
		pSwapChain->ResizeBuffers(0, screenWidth, screenHeight, DXGI_FORMAT_UNKNOWN, 0);
		CreateRenderTarget();
		CreateDepthBuffer();
	}

	const float color[3] = { 0.9, 1, 1 }; // CLEAR COLOR
	pd3dDeviceContext->ClearRenderTargetView(mainRenderTargetView, color);
	pd3dDeviceContext->OMSetRenderTargets(1, &mainRenderTargetView, depthStencilView);
	pd3dDeviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void RenderSystem::EndScene()
{
	// Present
	HRESULT hr = pSwapChain->Present(0, 0); // Without vsync
	SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
}

void RenderSystem::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	CleanupRasterizerState();
	CleanupDepthStencilState();
	CleanupDepthBuffer();
	CleanupRenderTarget();
	CleanupDeviceD3D();
}

// HELPER FUNCTIONS
bool RenderSystem::CreateDeviceD3D(HWND hWnd)
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
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice, &featureLevel, &pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice, &featureLevel, &pd3dDeviceContext);
	if (res != S_OK)
		return false;

	return true;
}

void RenderSystem::CleanupDeviceD3D()
{
	if (pd3dDeviceContext)
	{
		pd3dDeviceContext->Release();
		pd3dDeviceContext = nullptr;
	}
	if (pd3dDevice)
	{
		pd3dDevice->Release();
		pd3dDevice = nullptr;
	}
	if (pSwapChain)
	{
		pSwapChain->Release();
		pSwapChain = nullptr;
	}
}

bool RenderSystem::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	HRESULT result = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (FAILED(result))
		return false;

	result = pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
	if (FAILED(result))
		return false;

	pBackBuffer->Release();
	pBackBuffer = nullptr;

	return true;
}

void RenderSystem::CleanupRenderTarget()
{
	if (mainRenderTargetView)
	{
		mainRenderTargetView->Release();
		mainRenderTargetView = nullptr;
	}
}

bool RenderSystem::CreateDepthStencilState()
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

	HRESULT result = pd3dDevice->CreateDepthStencilState(&dsd, &depthStencilState);
	if (FAILED(result))
		return false;

	return true;
}

void RenderSystem::CleanupDepthStencilState()
{
	if (depthStencilState)
	{
		depthStencilState->Release();
		depthStencilState = nullptr;
	}
}

bool RenderSystem::CreateDepthBuffer()
{
	D3D11_TEXTURE2D_DESC dbd;
	ZeroMemory(&dbd, sizeof(dbd));
	dbd.Width = screenWidth;
	dbd.Height = screenHeight;
	dbd.MipLevels = 1;
	dbd.ArraySize = 1;
	dbd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dbd.SampleDesc.Count = 1;
	dbd.SampleDesc.Quality = 0;
	dbd.Usage = D3D11_USAGE_DEFAULT;
	dbd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dbd.CPUAccessFlags = 0;
	dbd.MiscFlags = 0;

	HRESULT result = pd3dDevice->CreateTexture2D(&dbd, nullptr, &depthStencilBuffer);
	if (FAILED(result))
		return false;

	pd3dDeviceContext->OMSetDepthStencilState(depthStencilState, 1);

	// Stencil view descriptioon
	D3D11_DEPTH_STENCIL_VIEW_DESC svd;
	ZeroMemory(&svd, sizeof(svd));
	svd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	svd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	svd.Texture2D.MipSlice = 0;

	result = pd3dDevice->CreateDepthStencilView(depthStencilBuffer, &svd, &depthStencilView);
	if (FAILED(result))
		return false;

	return true;
}

void RenderSystem::CleanupDepthBuffer()
{
	if (depthStencilView) {
		depthStencilView->Release();
		depthStencilView = nullptr;
	}
	if (depthStencilBuffer) {
		depthStencilBuffer->Release();
		depthStencilBuffer = nullptr;
	}
}

bool RenderSystem::CreateRasterizerState()
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

	HRESULT result = pd3dDevice->CreateRasterizerState(&rd, &rasterState);
	if (FAILED(result))
		return false;

	pd3dDeviceContext->RSSetState(rasterState);

	return true;
}

void RenderSystem::CleanupRasterizerState()
{
	if (rasterState)
	{
		rasterState->Release();
		rasterState = nullptr;
	}
}

void RenderSystem::InitializeViewport()
{
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	pd3dDeviceContext->RSSetViewports(1, &viewport);
}

void RenderSystem::InitializeMatrices()
{
	float fieldOfView, screenAspect, screenNear, screenDepth;
	fieldOfView = 3.14159f / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;
	screenNear = 0.3f;
	screenDepth = 1000.0f;

	projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	worldMatrix = XMMatrixIdentity();

	orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);
}

void RenderSystem::SetBackBufferRenderTarget()
{
	pd3dDeviceContext->OMSetRenderTargets(1, &mainRenderTargetView, depthStencilView);
}

void RenderSystem::ResetViewport()
{
	pd3dDeviceContext->RSSetViewports(1, &viewport);
}