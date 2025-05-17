#pragma once

#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;

// The goal of the render architecture is to be able to call a function Render() to the main screen or to a stored texture.
// Stored textures must accommodate custom width/height and work as a shader input.
class RenderTexture
{
public:
	RenderTexture();

	bool Initialize(ID3D11Device*, int, int);
	void Shutdown();
	
	void SetRenderTarget(ID3D11DeviceContext*);
	void ClearRenderTarget(ID3D11DeviceContext*, float, float, float, float);
	ID3D11ShaderResourceView* GetShaderResourceView();

	void BeginTextureScene();

private:
	bool CreateTextureRenderTarget(UINT, UINT);
	void BeginTextureScene(UINT, UINT);

private:
	ID3D11Texture2D* m_renderTargetTexture = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
	ID3D11ShaderResourceView* m_shaderResourceView = nullptr;

	ID3D11Texture2D* m_depthStencilBuffer = nullptr;
	ID3D11DepthStencilView* m_depthStencilView = nullptr;

	D3D11_VIEWPORT m_viewport = {};

	XMMATRIX m_projectionMatrix = {};
	XMMATRIX m_orthoMatrix = {};

	UINT m_textureWidth = 0, m_textureHeight = 0;
};