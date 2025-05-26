#pragma once

#include "d3d11.h"
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class RenderTarget
{
public:
    RenderTarget() = default;

	bool Initialize(ID3D11Device* device, UINT textureWidth, UINT textureHeight)
	{
        m_target.Reset();
        m_resource.Reset();
        
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

        ComPtr<ID3D11Texture2D> texture;
        HRESULT result = device->CreateTexture2D(&td, nullptr, &texture);
        if (FAILED(result))
            return false;

        // RTV
        device->CreateRenderTargetView(texture.Get(), nullptr, m_target.GetAddressOf());

        // SRV
        device->CreateShaderResourceView(texture.Get(), nullptr, m_resource.GetAddressOf());

        return true;
	}

private:
    ComPtr<ID3D11RenderTargetView> m_target;
    ComPtr<ID3D11ShaderResourceView> m_resource;
    UINT m_numViews = 1; // Number of resource views
};