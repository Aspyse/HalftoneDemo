#pragma once

#include "d3d11.h"
#include "DirectXMath.h"

struct RenderTarget
{
	ID3D11RenderTargetView* target = nullptr;
    ID3D11ShaderResourceView* resource = nullptr;
    UINT numViews = 1; // Number of resource views

    RenderTarget() = default;

	RenderTarget(ID3D11Device* device, UINT textureWidth, UINT textureHeight)
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
            return;

        // RTV
        device->CreateRenderTargetView(texture, nullptr, &target);

        // SRV
        device->CreateShaderResourceView(texture, nullptr, &resource);

        texture->Release();
        texture = nullptr;
	}

    ~RenderTarget()
    {
        if (target)
        {
            target->Release();
            target = nullptr;
        }

        if (resource)
        {
            resource->Release();
            resource = nullptr;
        }
    }
};