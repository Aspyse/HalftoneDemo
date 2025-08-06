#include "lighting_pass.h"

bool LightingPass::InitializeConstantBuffer(ID3D11Device* device)
{
	AddCB<MatrixBufferType>(device);
	AddCB<LightBufferType>(device);

	return true;
}

void LightingPass::Begin(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
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

	HRESULT result = device->CreateSamplerState(&shadowDesc, &shadowSampler);
	if (FAILED(result))
		return;

	deviceContext->PSSetSamplers(1, 1, &shadowSampler);
}

std::vector<RenderPass::ParameterControl> LightingPass::GetParameters()
{
	return {
		{ "Albedo", RenderPass::WidgetType::RENDER_TARGET, std::ref(m_inputs[0]) },
		{ "Normal + Roughness", RenderPass::WidgetType::RENDER_TARGET, std::ref(m_inputs[1]) },
		{ "Depth", RenderPass::WidgetType::RENDER_TARGET, std::ref(m_inputs[2]) },
		{ "Shadow Map", RenderPass::WidgetType::RENDER_TARGET, std::ref(m_inputs[3]) },
		//{ "Cel Threshold", RenderPass::WidgetType::FLOAT, std::ref(m_lightBuffer.celThreshold) }
		{ "Light Color", RenderPass::WidgetType::FLOAT3, std::ref(m_lightBuffer.lightColor) },
		//{ "Ambient Color", RenderPass::WidgetType::COLOR, std::ref(m_lightBuffer.ambientColor) },
	};
}

void LightingPass::Update(ID3D11DeviceContext* deviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMMATRIX lightViewProj, XMVECTOR lightDirection, XMFLOAT3 clearColor, UINT width, UINT height)
{
	XMVECTOR lightDirectionVecVS = XMVector3TransformNormal(lightDirection, viewMatrix);
	XMFLOAT3 lightDirectionVS;
	XMStoreFloat3(&lightDirectionVS, lightDirectionVecVS);

	XMMATRIX invProj = XMMatrixInverse(nullptr, projectionMatrix);
	XMMATRIX invView = XMMatrixInverse(nullptr, viewMatrix);
	
	m_matrixBuffer.invProj = XMMatrixTranspose(invProj);
	m_matrixBuffer.invView = XMMatrixTranspose(invView);

	m_lightBuffer.lightViewProj = XMMatrixTranspose(lightViewProj);
	m_lightBuffer.lightDirectionVS = lightDirectionVS;
	m_lightBuffer.ambientColor = clearColor;

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	deviceContext->Map(m_constantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	std::memcpy(mapped.pData, &m_matrixBuffer, sizeof(m_matrixBuffer));
	deviceContext->Unmap(m_constantBuffers[0].Get(), 0);

	deviceContext->Map(m_constantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	std::memcpy(mapped.pData, &m_lightBuffer, sizeof(m_lightBuffer));
	deviceContext->Unmap(m_constantBuffers[1].Get(), 0);
}