#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

class RenderSystem
{
public:
	RenderSystem();
	RenderSystem(const RenderSystem&);
	~RenderSystem();

	bool Initialize(int, int, bool, HWND, bool, float, float);
	void Shutdown();
	bool Frame();

	void BeginScene(float, float, float, float);
	void EndScene();

private:
	bool CreateDeviceD3D(HWND);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();

private:

};