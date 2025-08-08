#include "crosshatch_pass.h"

bool CrosshatchPass::InitializeConstantBuffer(ID3D11Device* device)
{
	AddCB<CrosshatchBufferType>(device);

	return true;
}

std::vector<RenderPass::ParameterControl> CrosshatchPass::GetParameters()
{
	return {
		{ "Normal Texture", RenderPass::WidgetType::RENDER_TARGET, std::ref(m_inputs[0]) },
		{ "Thickness Multiplier", RenderPass::WidgetType::FLOAT, std::ref(m_crosshatchBuffer.thicknessMul) },
		{ "Frequency Multiplier", RenderPass::WidgetType::FLOAT, std::ref(m_crosshatchBuffer.topoFreqMul) },
		{ "Threshold A", RenderPass::WidgetType::FLOAT, std::ref(m_crosshatchBuffer.thresholdA) },
		{ "Threshold B", RenderPass::WidgetType::FLOAT, std::ref(m_crosshatchBuffer.thresholdB) },
		{ "Stroke Color", RenderPass::WidgetType::COLOR, std::ref(m_crosshatchBuffer.inkColor) },
		{ "Hatch Angle", RenderPass::WidgetType::ANGLE, std::ref(m_crosshatchBuffer.hatchAngle) },
		{ "Is Feather", RenderPass::WidgetType::CHECKBOX, std::ref(m_crosshatchBuffer.isFeather) }
	};
}

void CrosshatchPass::Update(ID3D11DeviceContext* deviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMMATRIX lightViewProj, XMVECTOR lightDirection, XMFLOAT3 clearColor, UINT width, UINT height)
{
	XMVECTOR lightDirectionVecVS = XMVector3TransformNormal(lightDirection, viewMatrix);
	XMFLOAT3 lightDirectionVS;
	XMStoreFloat3(&lightDirectionVS, lightDirectionVecVS);

	m_crosshatchBuffer.lightDirectionVS = lightDirectionVS;
	m_crosshatchBuffer.clearColor = clearColor;

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	deviceContext->Map(m_constantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	std::memcpy(mapped.pData, &m_crosshatchBuffer, sizeof(m_crosshatchBuffer));
	deviceContext->Unmap(m_constantBuffers[0].Get(), 0);
}