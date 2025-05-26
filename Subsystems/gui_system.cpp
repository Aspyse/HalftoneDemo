#include "gui_system.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <tchar.h>

GuiSystem::GuiSystem() {}
GuiSystem::GuiSystem(const GuiSystem& other) {}
GuiSystem::~GuiSystem() {}

bool GuiSystem::Initialize(HWND hwnd)
{
	// Create application window
	ImGui_ImplWin32_EnableDpiAwareness();

	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	m_io = &ImGui::GetIO(); (void)m_io;
	m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// Setup ImGui style
	ImGui::StyleColorsDark();

	// Setup backends
	ImGui_ImplWin32_Init(hwnd);

	return true; // TEMP
}

void GuiSystem::Shutdown()
{
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

bool GuiSystem::Frame(RenderParameters& rParams)
{
	// Start the ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	static char filename[256] = "";

	// Show window
	{
		ImGui::Begin("Test Window");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_io->Framerate, m_io->Framerate);

		ImGui::ColorEdit3("Albedo Color", rParams.albedoColor);
		ImGui::DragFloat("Cel Threshold", &rParams.celThreshold, 0.002f, 0, 1);
		ImGui::DragFloat("Roughness", &rParams.roughness, 0.002f, 0, 1);
		ImGui::DragInt("Halftone Dot Size", &rParams.halftoneDotSize, 0.002f, 1, 50);

		ImGui::Separator();

		ImGui::DragFloat3("Light Direction", rParams.lightDirection, 0.004f);
		ImGui::ColorEdit3("Clear Color", rParams.clearColor);
		ImGui::DragFloat("Ambient Strength", &rParams.ambientStrength, 0.002f, 0, 1);

		ImGui::Separator();

		
		//if (ImGui::InputText("Filename", filename, IM_ARRAYSIZE(filename), ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Load Model"))
			// reset model

		ImGui::End();
	}

	ImGui::Render();

	return true;
}