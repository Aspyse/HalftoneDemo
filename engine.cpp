#include "engine.h"
#include "gui_system.h"
#include "input_system.h"

Engine::Engine() {}
Engine::Engine(const Engine& other) {}
Engine::~Engine() {}

bool Engine::Initialize()
{
	bool result;

	input_system = new InputSystem;
	input_system->Initialize();

	gui_system = new GuiSystem;
	gui_system->Initialize(input_system);

	//render_system = new RenderSystem;
	//render_system->Initialize();

	return true; // TEMP
}

void Engine::Shutdown()
{
	if (gui_system)
	{
		gui_system->Shutdown();
		delete gui_system;
		gui_system = nullptr;
	}
	if (input_system)
	{
		delete input_system;
		input_system = nullptr;
	}

	return;
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

	if (input_system->IsKeyDown(VK_ESCAPE))
		return false;

	result = gui_system->Frame();
	if (!result)
		return false;

	return true;
}