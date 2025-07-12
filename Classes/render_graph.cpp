#include "render_graph.h"

using namespace DirectX;

RenderGraph::RenderGraph(ID3D11Device* device, ID3D11DeviceContext* context, UINT screenWidth, UINT screenHeight)
{
	m_device = device;
	m_deviceContext = context;
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
}

void RenderGraph::DebugInit(std::shared_ptr<RenderTarget> gBuffer, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv)
{
	// Custom samplers
	ID3D11SamplerState* shadowSampler;
	D3D11_SAMPLER_DESC shadowDesc;
	ZeroMemory(&shadowDesc, sizeof(shadowDesc));
	shadowDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	shadowDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	shadowDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	shadowDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	shadowDesc.MipLODBias = 0.0f;
	shadowDesc.MaxAnisotropy = 1;
	shadowDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	shadowDesc.BorderColor[0] = 1.0f;
	shadowDesc.BorderColor[1] = 1.0f;
	shadowDesc.BorderColor[2] = 1.0f;
	shadowDesc.BorderColor[3] = 1.0f;
	shadowDesc.MinLOD = 0;
	shadowDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT result = m_device->CreateSamplerState(&shadowDesc, &shadowSampler);
	if (FAILED(result))
		return;
	
	/* RENDER PASSES */

	auto lightingPass = std::make_unique<LightingPass>();
	lightingPass->Initialize(m_device, m_screenWidth, m_screenHeight);
	lightingPass->Begin = [this, shadowSampler]()
		{
			m_deviceContext->PSSetSamplers(1, 1, &shadowSampler);
		};
	m_passes.push_back(std::move(lightingPass)); // pass 0


	auto halftonePass = std::make_unique<HalftonePass>();
	halftonePass->Initialize(m_device, m_screenWidth, m_screenHeight);
	m_passes.push_back(std::move(halftonePass));


	/*
	auto blendPass = std::make_unique<BlendPass>();
	blendPass->Initialize(m_device, L"Shaders/blend.ps");
	m_passes.push_back(std::move(blendPass)); // pass 2
	*/


	m_passes[0]->SetInput(gBuffer);
	m_passes[1]->SetInput(m_passes[0]->GetOutput());
	m_passes[0]->WrapOutput(rtv, dsv);
}

void RenderGraph::SetParameters(RenderParameters& rParams, XMVECTOR lightDirectionVec, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMMATRIX lightViewProj)
{
	/* CONSTANT BUFFER PARAMETERS*/
	XMFLOAT3 lightColor = XMFLOAT3(4.0f, 4.0f, 4.0f);
	XMVECTOR lightDirectionVecVS = XMVector3TransformNormal(lightDirectionVec, viewMatrix);
	XMFLOAT3 lightDirectionVS;
	XMStoreFloat3(&lightDirectionVS, lightDirectionVecVS);

	if (auto* lp = dynamic_cast<LightingPass*>(m_passes[0].get()))
		lp->SetShaderParameters(m_deviceContext, projectionMatrix, viewMatrix, lightViewProj, lightDirectionVS, lightColor, rParams.clearColor, rParams.ambientStrength, rParams.celThreshold);

	if (auto* hp = dynamic_cast<HalftonePass*>(m_passes[1].get()))
	{
		float offsets[3] = { 1.0f, 1.0f, 1.0f };
		hp->SetShaderParameters(m_deviceContext, rParams.inkColor, m_screenWidth / rParams.halftoneDotSize, m_screenHeight / rParams.halftoneDotSize, false, offsets);
	}

	/*
	if (auto* sp = dynamic_cast<SobelPass*>(m_passes[1].get()))
		sp->SetShaderParameters(m_deviceContext, m_screenWidth, m_screenHeight, 1, rParams.edgeThreshold, rParams.inkColor, rParams.clearColor);
	*/
}

void RenderGraph::Render(ID3D11SamplerState* sampler, RenderParameters& rParams)
{
	/* FORWARD RENDER */
	for (auto& pass : m_passes)
	{
		m_deviceContext->PSSetSamplers(0, 1, &sampler);
		pass->Render(m_deviceContext, rParams.clearColor);

		ID3D11RenderTargetView* nullRTV = nullptr;
		m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
	}
}

void RenderGraph::Resize(UINT width, UINT height)
{
	m_screenWidth = width;
	m_screenHeight = height;
	for (auto& p: m_passes)
	{
		p->GetOutput()->Initialize(m_device, width, height);
	}
}