#include "engine.h"
#include "input_system.h"
#include "gui_system.h"
#include "render_system.h"

Engine::Engine() {}
Engine::Engine(const Engine& other) {}
Engine::~Engine() {}

bool Engine::Initialize()
{
	bool result;

	m_inputSystem = new InputSystem;
	m_inputSystem->Initialize();

	m_guiSystem = new GuiSystem;
	m_guiSystem->Initialize(m_inputSystem);

	HWND hwnd = m_guiSystem->GetHWND();
	WNDCLASSEXW wc = m_guiSystem->GetWC();

	m_renderSystem = new RenderSystem;
	m_renderSystem->Initialize(hwnd, wc, m_inputSystem);

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

	if (m_inputSystem->IsKeyDown(VK_ESCAPE))
		return false;

	result = m_guiSystem->Frame();
	if (!result)
		return false;

	result = m_renderSystem->Render();
	if (!result)
		return false;

	return true;
}