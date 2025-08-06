#pragma once

#include "d3d11.h"
#include <wrl/client.h>
#include <vector>

using std::vector;
using Microsoft::WRL::ComPtr;

class RenderTarget
{
public:
    RenderTarget() = default;
    RenderTarget(ComPtr<ID3D11ShaderResourceView> srv) :
        m_resourceView(srv)
    {
        ComPtr<ID3D11Resource> res;
        m_resourceView->GetResource(&res);

        res.As(&m_texture);
    }

    void ClearTarget(ID3D11DeviceContext* deviceContext, float* clearColor)
    {
        deviceContext->ClearRenderTargetView(m_target.Get(), clearColor);
        if (m_dsv)
            deviceContext->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

	bool Initialize(ID3D11Device* device, UINT textureWidth, UINT textureHeight)
	{
        m_target.Reset();
        m_resourceView.Reset();
        
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

        //ComPtr<ID3D11Texture2D> texture;
        HRESULT result = device->CreateTexture2D(&td, nullptr, m_texture.GetAddressOf());
        if (FAILED(result))
            return false;

        // RTV
        result = device->CreateRenderTargetView(m_texture.Get(), nullptr, m_target.GetAddressOf());
        if (FAILED(result))
            return false;

        // SRV
        result = device->CreateShaderResourceView(m_texture.Get(), nullptr, m_resourceView.GetAddressOf());
        if (FAILED(result))
            return false;

        return true;
	}

    // TODO: consider getting pointer-to-pointer
    ID3D11RenderTargetView* const* GetTarget()
    {
        return m_target.GetAddressOf();
    }

    ID3D11Texture2D* GetResource()
    {
        return m_texture.Get();
    }

    ID3D11ShaderResourceView* const* GetResourceView()
    {
        return m_resourceView.GetAddressOf();
    }

private:
    ComPtr<ID3D11Texture2D> m_texture;
    ComPtr<ID3D11RenderTargetView> m_target;
    ComPtr<ID3D11DepthStencilView> m_dsv;
    ComPtr<ID3D11ShaderResourceView> m_resourceView;
};