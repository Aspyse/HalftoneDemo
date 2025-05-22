#include "render_system.h"
#include "imgui_impl_dx11.h"

RenderSystem::RenderSystem() {};
RenderSystem::RenderSystem(const RenderSystem&) {};
RenderSystem::~RenderSystem() {};

// MENU VALUES
float* RenderSystem::LightDirection()
{
	return m_lightDirection;
}

float* RenderSystem::ClearColor()
{
	return m_clearColor;
}

float& RenderSystem::AmbientStrength()
{
	return m_ambientStrength;
}

float& RenderSystem::CelThreshold()
{
	return m_celThreshold;
}

float* RenderSystem::AlbedoColor()
{
	return m_albedoColor;
}

float& RenderSystem::Roughness()
{
	return m_roughness;
}

bool RenderSystem::ResetModel(const char* filename)
{
	ModelClass* temp_model = new ModelClass;
	bool result = temp_model->Initialize(m_device, filename);
	if (!result)
	{
		if (temp_model)
		{
			delete temp_model;
			temp_model = nullptr;
		}
		return false;
	}

	if (m_model)
	{
		delete m_model;
		m_model = nullptr;
	}

	m_model = temp_model;
	return true;
}



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
	InitializeMatrices();

	m_camera = new CameraClass;
	m_camera->SetPosition(0.0f, 0.1f, -1.0f); // TEMP VALUES
	m_camera->SetRotation(0.0f, 0.0f, 0.0f);
	m_camera->SetOrbitPosition(0.0f, 0.1f, 0.0f);
	
	m_model = new ModelClass;
	m_model->Initialize(m_device, "Models/bun_zipper.ply"); // TODO: Handle failure
	
	m_geometryPass = new GeometryPass;
	m_geometryPass->Initialize(m_device, m_screenWidth, m_screenHeight);

	m_lightingShader = new LightingShader;
	m_lightingShader->Initialize(m_device, L"Shaders/base.ps");

	m_halftoneRT = new RenderTexture;
	//m_halftoneRT->Initialize(m_device, m_screenWidth/m_halftoneDotSize, m_screenHeight/m_halftoneDotSize);
	m_halftoneRT->Initialize(m_device, m_screenWidth, m_screenHeight);

	m_halftoneShader = new HalftoneShader;
	m_halftoneShader->Initialize(m_device, L"Shaders/halftone.ps", "HalftonePixelShader");

	return true;
}

void RenderSystem::Shutdown()
{
	if (m_lightingShader)
	{
		m_lightingShader->Shutdown();
		delete m_lightingShader;
		m_lightingShader = nullptr;
	}
	if (m_geometryPass)
	{
		m_geometryPass->Shutdown();
		delete m_geometryPass;
		m_geometryPass = nullptr;
	}
	if (m_model)
	{
		m_model->Shutdown();
		delete m_model;
		m_model = nullptr;
	}
	if (m_camera)
	{
		delete m_camera;
		m_camera = nullptr;
	}

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


bool RenderSystem::Render(InputSystem* inputHandle)
{
	// Resize
	if (m_screenWidth != inputHandle->GetResizeWidth() || m_screenHeight != inputHandle->GetResizeHeight())
	{
		m_screenWidth = inputHandle->GetResizeWidth();
		m_screenHeight = inputHandle->GetResizeHeight();

		CleanupRenderTarget();
		CleanupDepthBuffer();
		m_swapChain->ResizeBuffers(0, m_screenWidth, m_screenHeight, DXGI_FORMAT_UNKNOWN, 0);
		m_geometryPass->InitializeGBuffer(m_device, m_screenWidth, m_screenHeight);
		CreateRenderTarget();
		CreateDepthBuffer();

		ResetViewport(m_screenWidth, m_screenHeight);
		InitializeMatrices();
		//m_halftoneRT->Initialize(m_device, m_screenWidth / m_halftoneDotSize, m_screenHeight / m_halftoneDotSize);
		m_halftoneRT->Initialize(m_device, m_screenWidth, m_screenHeight);
	}

	// Update camera
	m_camera->Frame(inputHandle);

	XMMATRIX viewMatrix;
	m_camera->GetViewMatrix(viewMatrix);

	// Update model
	m_model->Render(m_deviceContext);



	// GEOMETRY PASS
	XMFLOAT3 lightDirectionF = XMFLOAT3(m_lightDirection[0], m_lightDirection[1], m_lightDirection[2]);
	XMVECTOR lightDirectionVec = XMVector3Normalize(XMLoadFloat3(&lightDirectionF));
	m_geometryPass->RenderShadow(m_deviceContext, m_model->GetIndexCount(), lightDirectionVec);

	// Reset after viewport is set to shadowmap size
	ResetViewport(m_screenWidth, m_screenHeight);
	
	XMFLOAT3 albedoColor = XMFLOAT3(m_albedoColor[0], m_albedoColor[1], m_albedoColor[2]);
	m_geometryPass->SetShaderParameters(m_deviceContext, m_worldMatrix, viewMatrix, m_projectionMatrix, albedoColor, m_roughness);
	m_geometryPass->Render(m_deviceContext, m_model->GetIndexCount(), m_clearColor);
	ClearRenderTargets();


	
	// LIGHTING PASS
	ID3D11ShaderResourceView* gBuffer[] = {
		m_geometryPass->GetGBuffer(0),
		m_geometryPass->GetGBuffer(1),
		m_geometryPass->GetGBuffer(2),
		m_geometryPass->GetShadowMap()
	};

	m_deviceContext->PSSetShaderResources(0, 4, gBuffer);

	ID3D11RenderTargetView* rtv = m_halftoneRT->GetTarget();
	m_deviceContext->OMSetRenderTargets(1, &rtv, nullptr);
	//BeginScene();
	ResetViewport(m_screenWidth, m_screenHeight);
	//ResetViewport(m_screenWidth / m_halftoneDotSize, m_screenHeight / m_halftoneDotSize);

	XMFLOAT3 lightColor = XMFLOAT3(4.0f, 4.0f, 4.0f);
	XMFLOAT3 ambientColor = XMFLOAT3(m_clearColor[0] * m_ambientStrength, m_clearColor[1] * m_ambientStrength, m_clearColor[2] * m_ambientStrength);
	float celThreshold = 0.0f;
	m_lightingShader->SetShaderParameters(m_deviceContext, m_projectionMatrix, viewMatrix, m_geometryPass->GetLightViewProj(), lightDirectionVec, lightColor, ambientColor, celThreshold);
	m_lightingShader->Render(m_deviceContext);
	ClearRenderTargets();

	
	
	// POST-PROCESS (HALFTONE)
	ID3D11ShaderResourceView* srv = m_halftoneRT->GetResource();
	m_deviceContext->PSSetShaderResources(0, 1, &srv);

	// Clear the buffers to begin the scene
	BeginScene();
	ResetViewport(m_screenWidth, m_screenHeight);

	m_halftoneShader->SetShaderParameters(m_deviceContext, m_halftoneDotSize);
	m_halftoneShader->Render(m_deviceContext); 
	

	// Render GUI
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // TODO: consider decoupling with GUI

	// Present the rendered scene to the screen
	EndScene();
	return true; // TEMP
}

void RenderSystem::ClearRenderTargets()
{
	ID3D11RenderTargetView* nullRTV = nullptr;
	m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void RenderSystem::BeginScene()
{
	// Handle minimize or screen lock
	if (m_isSwapChainOccluded && m_swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
	{
		::Sleep(10);
		return;
	}
	m_isSwapChainOccluded = false;

	m_deviceContext->ClearRenderTargetView(m_renderTargetView, m_clearColor);
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void RenderSystem::EndScene()
{
	// Present
	HRESULT hr = m_swapChain->Present(0, 0); // Without vsync
	m_isSwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
}

// HELPER FUNCTIONS
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

	HRESULT result = m_device->CreateTexture2D(&dbd, nullptr, &m_depthStencilBuffer);
	if (FAILED(result))
		return false;

	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	// Stencil view descriptioon
	D3D11_DEPTH_STENCIL_VIEW_DESC svd;
	ZeroMemory(&svd, sizeof(svd));
	svd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	svd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	svd.Texture2D.MipSlice = 0;

	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &svd, &m_depthStencilView);
	if (FAILED(result))
		return false;

	return true;
}

void RenderSystem::CleanupDepthBuffer()
{
	if (m_depthStencilView) {
		m_depthStencilView->Release();
		m_depthStencilView = nullptr;
	}
	if (m_depthStencilBuffer) {
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = nullptr;
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

void RenderSystem::InitializeMatrices()
{
	// TODO: consider moving to CameraClass
	float fieldOfView, screenAspect, screenNear, screenDepth;
	fieldOfView = 0.5f*(3.14159f / 4.0f);
	screenAspect = (float)m_screenWidth / (float)m_screenHeight;
	screenNear = 0.3f;
	screenDepth = 100.0f; // ADJUST

	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	m_worldMatrix = XMMatrixIdentity();

	m_orthoMatrix = XMMatrixOrthographicLH((float)m_screenWidth, (float)m_screenHeight, screenNear, screenDepth);
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