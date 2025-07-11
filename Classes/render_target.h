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

    void BindAsResource(ID3D11DeviceContext* deviceContext)
    {
        deviceContext->PSSetShaderResources(0, m_numViews, m_shaderResource);
    }

    void ClearTarget(ID3D11DeviceContext* deviceContext, float* clearColor)
    {
        deviceContext->ClearRenderTargetView(GetTarget(), clearColor);
        deviceContext->ClearDepthStencilView(m_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

    void BindAsTarget(ID3D11DeviceContext* deviceContext)
    {
        deviceContext->OMSetRenderTargets(1, m_target.GetAddressOf(), m_dsv);
    }

    bool SetTarget(ID3D11RenderTargetView* target, ID3D11DepthStencilView* dsv = nullptr)
    {
        m_target = target;
        m_dsv = dsv;
        return true;
    }

    /*
    bool SetResource(ID3D11ShaderResourceView* resource)
    {
        return SetResource(&resource, 1);
    }
    bool SetResource(ID3D11ShaderResourceView* const* resources, UINT numViews)
    {
        m_resources.clear();
        m_resourcePointers.clear();

        for (UINT i = 0; i < numViews; ++i)
        {
            if (resources[i])
                m_resources.emplace_back(resources[i]);
            else
                m_resources.push_back(nullptr);
            
            m_resourcePointers.push_back(m_resources.back().Get());
        }

        return true;
    }
    */

    bool SetResource(ID3D11ShaderResourceView* const* resource, UINT numViews)
    {
        m_shaderResource = resource;
        m_numViews = numViews;

        return true;
    }

	bool Initialize(ID3D11Device* device, UINT textureWidth, UINT textureHeight)
	{
        m_target.Reset();
        //m_resources.clear();
        //m_resourcePointers.clear();
        
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
        result = device->CreateRenderTargetView(texture.Get(), nullptr, m_target.GetAddressOf());
        if (FAILED(result))
            return false;

        // SRV
        //result = device->CreateShaderResourceView(texture.Get(), nullptr, &tempResource);
        //result = device->CreateShaderResourceView(texture.Get(), nullptr, &m_shaderResource);
        //if (FAILED(result))
            return false;

        //m_resources.push_back(tempResource);
        //m_resourcePointers.push_back(tempResource.Get());

        return true;
	}

    // TODO: consider getting pointer-to-pointer
    ID3D11RenderTargetView* GetTarget()
    {
        return m_target.Get();
    }

    ID3D11DepthStencilView* GetDSV()
    {
        return m_dsv;
    }

    /*
    ID3D11ShaderResourceView* const* GetResource()
    {
        return m_resourcePointers.data();
    }
    */

    UINT GetNumViews()
    {
        //return static_cast<UINT>(m_resourcePointers.size());
        return m_numViews = 0;
    }

private:
    ComPtr<ID3D11RenderTargetView> m_target;
    ID3D11DepthStencilView* m_dsv = nullptr;
    //vector<ComPtr<ID3D11ShaderResourceView>> m_resources;
    //vector<ID3D11ShaderResourceView*> m_resourcePointers;
    ID3D11ShaderResourceView* const* m_shaderResource = nullptr;
    UINT m_numViews = 0;
};