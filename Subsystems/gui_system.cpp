#include "gui_system.h"
#include "render_system.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <tchar.h>

GuiSystem::GuiSystem() {
	ZeroMemory(&m_wc, sizeof(m_wc));
	m_wc.cbSize = sizeof(WNDCLASSEXW);
}

GuiSystem::GuiSystem(const GuiSystem& other)
{
	ZeroMemory(&m_wc, sizeof(m_wc));
	m_wc.cbSize = sizeof(WNDCLASSEXW);
}
GuiSystem::~GuiSystem() {}

bool GuiSystem::Initialize(InputSystem* inputHandle)
{
	// Create application window
	ImGui_ImplWin32_EnableDpiAwareness();
	m_wc = { sizeof(m_wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Halftone Demo", nullptr };
	::RegisterClassExW(&m_wc);
	m_hwnd = ::CreateWindow(m_wc.lpszClassName, L"Halftone Demo", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, m_wc.hInstance, nullptr);

	// Store instance in the window user data
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)inputHandle);

	// Show window
	::ShowWindow(m_hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(m_hwnd);

	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	m_io = &ImGui::GetIO(); (void)m_io;
	m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// Setup ImGui style
	ImGui::StyleColorsDark();

	// Setup backends
	ImGui_ImplWin32_Init(m_hwnd);

	return true; // TEMP
}

void GuiSystem::Shutdown()
{
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	::DestroyWindow(m_hwnd);
	::UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
}

bool GuiSystem::Frame(RenderSystem* renderSystem)
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

		ImGui::DragFloat3("Light Direction", renderSystem->LightDirection(), 0.004f);
		ImGui::ColorEdit3("Clear Color", renderSystem->ClearColor());
		ImGui::DragFloat("Ambient Strength", &renderSystem->AmbientStrength(), 0.002f, 0, 1);
		ImGui::DragFloat("Cel Threshold", &renderSystem->CelThreshold(), 0.002f, 0, 1);

		ImGui::Separator();

		
		if (ImGui::InputText("Filename", filename, IM_ARRAYSIZE(filename), ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Load Model"))
			renderSystem->ResetModel(filename);

		ImGui::End();
	}

	ImGui::Render();

	return true;
}

HWND GuiSystem::GetHWND()
{
	return m_hwnd;
}

WNDCLASSEXW GuiSystem::GetWC()
{
	return m_wc;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	InputSystem* inputHandle = (InputSystem*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (inputHandle && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	if (inputHandle)
		return inputHandle->MessageHandler(hWnd, msg, wParam, lParam);
	
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}