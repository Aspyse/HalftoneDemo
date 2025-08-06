#include "effect.h"

using namespace DirectX;

Effect::Effect(ID3D11Device* device, ID3D11DeviceContext* context)
{
	m_device = device;
	m_deviceContext = context;
}

void Effect::Initialize(std::vector<ComPtr<ID3D11ShaderResourceView>> g_buffer, UINT screenWidth, UINT screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	
	m_targets["g_albedo"] = std::make_unique<RenderTarget>(g_buffer[0]);
	m_targets["g_normal"] = std::make_unique<RenderTarget>(g_buffer[1]);
	m_targets["g_depth"] = std::make_unique<RenderTarget>(g_buffer[2]);
	m_targets["g_shadow"] = std::make_unique<RenderTarget>(g_buffer[3]);
	
	for (const auto& p : m_passes)
	{
		p->Initialize(m_device, m_screenWidth, m_screenHeight);
		for (auto o : p->outputs())
		{
			m_targets[o] = std::make_unique<RenderTarget>();
			m_targets[o]->Initialize(m_device, m_screenWidth, m_screenHeight);
		}
	}
}

const std::unordered_map<std::string, std::unique_ptr<RenderTarget>>& Effect::GetTargets() const
{
	return m_targets;
}

// Used for GUI parameters
const vector<std::unique_ptr<RenderPass>>& Effect::GetPasses() const
{
	return m_passes;
}

void Effect::AddPass(std::unique_ptr<RenderPass> renderPass)
{
	for (auto o : renderPass->outputs())
	{
		m_targets[o] = std::make_unique<RenderTarget>();
		m_targets[o]->Initialize(m_device, m_screenWidth, m_screenHeight);
	}
	
	renderPass->Initialize(m_device, m_screenWidth, m_screenHeight);
	m_passes.push_back(std::move(renderPass));
}

/*
void Effect::SetParameters(RenderParameters& rParams, XMVECTOR lightDirectionVec, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMMATRIX lightViewProj)
{
	// CONSTANT BUFFER PARAMETERS
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

	
	if (auto* sp = dynamic_cast<SobelPass*>(m_passes[1].get()))
		sp->SetShaderParameters(m_deviceContext, m_screenWidth, m_screenHeight, 1, rParams.edgeThreshold, rParams.inkColor, rParams.clearColor);
	
}
*/

void Effect::Update(XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMMATRIX lightViewProj, XMVECTOR lightDirection, XMFLOAT3 clearColor)
{
	for (const auto& pass : m_passes)
	{
		pass->Update(m_deviceContext, viewMatrix, projectionMatrix, lightViewProj, lightDirection, clearColor, m_screenWidth, m_screenHeight);
	}
}

void Effect::Render(ID3D11SamplerState* sampler)
{
	/* FORWARD RENDER */
	for (const auto& pass : m_passes)
	{
		m_deviceContext->PSSetSamplers(0, 1, &sampler);
		// TODO: figure out clear color
		float clearColor[3] = { 1, 1, 1 };
		
		int bindCtr = 0;
		auto inputs = pass->GetInputs();
		for (int i = 0; i < pass->GetInputs().size(); ++i)
		{
			auto index = inputs[i];
			if (index != "")
			{
				m_deviceContext->PSSetShaderResources(bindCtr, 1, m_targets[index]->GetResourceView());
				bindCtr++;
			}
		}

		for (auto out : pass->GetOutputs())
		{
			m_targets[out]->ClearTarget(m_deviceContext, clearColor);
			m_deviceContext->OMSetRenderTargets(1, m_targets[out]->GetTarget(), nullptr);
		}

		pass->Render(m_device, m_deviceContext, clearColor);
		
		ID3D11RenderTargetView* nullRTV = nullptr;
		m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
	}
	
}

void Effect::Resize(UINT width, UINT height)
{
	m_screenWidth = width;
	m_screenHeight = height;
	for (const auto& p : m_passes)
	{
		p->Initialize(m_device, m_screenWidth, m_screenHeight);
		for (auto o : p->outputs())
		{
			m_targets[o]->Initialize(m_device, m_screenWidth, m_screenHeight);
		}
	}
}