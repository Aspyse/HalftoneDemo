#pragma once

#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;

class RenderTexture
{
public:
	RenderTexture();
	RenderTexture(const RenderTexture&);
	~RenderTexture();

	bool Initialize(ID3D11Device*, UINT, UINT);
	void Shutdown();

	ID3D11RenderTargetView* GetTarget();
	ID3D11ShaderResourceView* GetResource();

private:
	ID3D11Texture2D* m_texture = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
	ID3D11ShaderResourceView* m_shaderResourceView = nullptr;

	UINT m_textureWidth = 0, m_textureHeight = 0;
};