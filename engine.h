#pragma once

#define WIN32_LEAN_AND_MEAN

#include "render_parameters.h"

#include "camera.h"

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
	RenderParameters m_renderParameters;
	std::vector<std::unique_ptr<ModelClass>> m_models;
	CameraClass* m_camera = nullptr;

	POINT m_lastMousePos = { 0, 0 };

	InputSystem* m_inputSystem = nullptr;
	GuiSystem* m_guiSystem = nullptr;
	RenderSystem* m_renderSystem = nullptr;

	WNDCLASSEXW m_wc = { 0 };
	HWND m_hwnd = nullptr;
};