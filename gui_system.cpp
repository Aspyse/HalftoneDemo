#include "gui_system.h"
#include "render_system.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <tchar.h>

GuiSystem::GuiSystem() {
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEXW);
};

GuiSystem::GuiSystem(const GuiSystem& other)
{
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEXW);
}
GuiSystem::~GuiSystem() {}

bool GuiSystem::Initialize(InputSystem* inputHandle)
{
	// Create application window
	ImGui_ImplWin32_EnableDpiAwareness();
	wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Halftone Demo", nullptr };
	::RegisterClassExW(&wc);
	hwnd = ::CreateWindow(wc.lpszClassName, L"Halftone Demo", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

	// Store instance in the window user data
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)inputHandle);

	// Show window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO(); (void)io;
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

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

	::DestroyWindow(hwnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

bool GuiSystem::Frame()
{
	// Start the ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Show window
	{
		ImGui::Begin("Test Window");
		ImGui::Text("Hello, world!");

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
		ImGui::End();
	}

	ImGui::Render();

	return true;
}

HWND GuiSystem::GetHWND()
{
	return hwnd;
}

WNDCLASSEXW GuiSystem::GetWC()
{
	return wc;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	InputSystem* input_handle = (InputSystem*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (input_handle && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	default:
		if (input_handle)
			return input_handle->MessageHandler(hWnd, msg, wParam, lParam);
		break;
	}

	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}