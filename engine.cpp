#include "engine.h"

Engine::Engine() {}
Engine::Engine(const Engine& other) {}
Engine::~Engine() {}

bool Engine::Initialize()
{
	const int width = 1280, height = 800;
	
	bool result;
	WNDCLASSEXW m_wc = { sizeof(m_wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Halftone Demo", nullptr };
	::RegisterClassExW(&m_wc);
	HWND m_hwnd = ::CreateWindow(m_wc.lpszClassName, L"Halftone Demo", WS_OVERLAPPEDWINDOW, 100, 100, width, height, nullptr, nullptr, m_wc.hInstance, nullptr);

	// Show window
	::ShowWindow(m_hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(m_hwnd);

	m_inputSystem = new InputSystem;
	m_inputSystem->Initialize();


	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)m_inputSystem);

	m_guiSystem = new GuiSystem;
	m_guiSystem->Initialize(m_hwnd);

	m_camera = new CameraClass;
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	m_camera->Initialize(30, aspect, 0.3f, 100.0f);
	m_camera->SetPosition(0.0f, 0.1f, -1.0f);
	m_camera->SetRotation(0.0f, 0.0f, 0.0f);
	m_camera->SetOrbitPosition(0.0f, 0.1f, 0.0f);

	m_renderSystem = new RenderSystem;
	m_renderSystem->Initialize(m_hwnd, m_wc);

	// FOR TESTING: create model
	auto model = std::make_unique<ModelClass>();
	model->Initialize(m_renderSystem->GetDevice(), "Models/bun_zipper.ply");
	m_models.push_back(std::move(model));

	return true; // TEMP
}

void Engine::Shutdown()
{
	if (m_renderSystem)
	{
		m_renderSystem->Shutdown();
		delete m_renderSystem;
		m_renderSystem = nullptr;
	}
	if (m_guiSystem)
	{
		m_guiSystem->Shutdown();
		delete m_guiSystem;
		m_guiSystem = nullptr;
	}
	if (m_inputSystem)
	{
		delete m_inputSystem;
		m_inputSystem = nullptr;
	}

	::DestroyWindow(m_hwnd);
	::UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
}

void Engine::Run()
{
	bool done = false, result;

	while (!done)
	{
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

		// Loop
		result = Frame();
		if (!result)
		{
			done = true;
		}
	}
}

bool Engine::Frame()
{
	bool result;

	// Handle ESC
	if (m_inputSystem->IsKeyDown(VK_ESCAPE))
		return false;

	// Handle GUI
	result = m_guiSystem->Frame(m_renderParameters); // pass data in
	if (!result)
		return false;

	// Update camera
	POINT newPos = m_inputSystem->GetMousePos();
	POINT delta = {
		newPos.x - m_lastMousePos.x,
		newPos.y - m_lastMousePos.y
	};
	m_lastMousePos = newPos;
	m_camera->Frame(delta, m_inputSystem->IsMiddleMouseDown(), m_inputSystem->IsKeyDown(VK_SHIFT), m_inputSystem->GetScrollDelta());

	// Render
	if (m_inputSystem->IsResizeDirty())
	{
		m_renderSystem->Resize(m_inputSystem->GetResizeWidth(), m_inputSystem->GetResizeHeight());
		m_camera->Resize(m_inputSystem->GetResizeWidth(), m_inputSystem->GetResizeHeight());
	}
	result = m_renderSystem->Render(m_renderParameters, m_camera->GetViewMatrix(), m_camera->GetProjectionMatrix(), m_models);
	if (!result)
		return false;

	return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Read io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	InputSystem* inputHandle = (InputSystem*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (inputHandle && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	if (inputHandle)
		return inputHandle->MessageHandler(hWnd, msg, wParam, lParam);

	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}