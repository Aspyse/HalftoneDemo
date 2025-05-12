#pragma once

#define WIN32_LEAN_AND_MEAN

#include "input_system.h"
#include "gui_system.h"
#include "render_system.h"

class Engine
{
public:
	Engine();
	Engine(const Engine&);
	~Engine();

	bool Initialize();
	void Shutdown();
	void Run();

private:
	bool Frame();

private:
	InputSystem* input_system = nullptr;
	GuiSystem* gui_system = nullptr;
	RenderSystem* render_system = nullptr;
};