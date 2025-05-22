#include "render_texture.h"

RenderTexture::RenderTexture() {}
RenderTexture::RenderTexture(const RenderTexture&) {}
RenderTexture::~RenderTexture() {}

ID3D11RenderTargetView* RenderTexture::GetTarget()
{
    return m_renderTargetView;
}

ID3D11ShaderResourceView* RenderTexture::GetResource()
{
    return m_shaderResourceView;
}

bool RenderTexture::Initialize(ID3D11Device* device, UINT textureWidth, UINT textureHeight)
{
    D3D11_TEXTURE2D_DESC td;
    ZeroMemory(&td, sizeof(td));
    td.Width = textureWidth;
    td.Height = textureHeight;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    ID3D11Texture2D* texture = nullptr;
    HRESULT result = device->CreateTexture2D(&td, nullptr, &texture);
    if (FAILED(result))
        return false;

    // RTV
    device->CreateRenderTargetView(texture, nullptr, &m_renderTargetView);

    // SRV
    device->CreateShaderResourceView(texture, nullptr, &m_shaderResourceView);

    texture->Release();
    texture = nullptr;

	return true;
}

void RenderTexture::Shutdown()
{

}