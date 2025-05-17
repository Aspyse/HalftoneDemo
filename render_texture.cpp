#include "render_texture.h"

RenderTexture::RenderTexture() {}

bool RenderTexture::Initialize(ID3D11Device* device, int textureWidth, int textureHeight)
{
	DXGI_FORMAT textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	m_textureWidth = textureWidth;
	m_textureHeight = textureHeight;

	return true;
}
