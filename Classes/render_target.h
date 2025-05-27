#pragma once

#include "d3d11.h"
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class RenderTarget
{
public:
    RenderTarget() = default;

    bool SetTarget(ID3D11RenderTargetView* target)
    {
        m_target = target;
        return true;
    }

    bool SetResource(ID3D11ShaderResourceView* resource, UINT numViews)
    {
        m_resource = resource;
        m_numViews = numViews;
        return true;
    }

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
        HRESULT result = device->CreateTexture2D(&td, nullptr, texture.GetAddressOf());
        if (FAILED(result))
            return false;

        // RTV
        device->CreateRenderTargetView(texture.Get(), nullptr, m_target.GetAddressOf());
        if (FAILED(result))
            return false;

        // SRV
        device->CreateShaderResourceView(texture.Get(), nullptr, m_resource.GetAddressOf());
        if (FAILED(result))
            return false;

        return true;
	}

    // TODO: consider getting pointer-to-pointer
    ID3D11RenderTargetView* GetTarget()
    {
        return m_target.Get();
    }

    ID3D11ShaderResourceView** GetResource()
    {
        return m_resource.GetAddressOf();
    }

    UINT GetNumViews()
    {
        return m_numViews;
    }

private:
    ComPtr<ID3D11RenderTargetView> m_target;
    ComPtr<ID3D11ShaderResourceView> m_resource;
    UINT m_numViews = 1; // Number of resource views
};