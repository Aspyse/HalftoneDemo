#include "halftone_pass.h"
#include <functional>

bool HalftonePass::InitializeConstantBuffer(ID3D11Device* device)
{
	AddCB<HalftoneBufferType>(device);

	return true;
}

std::vector<RenderPass::ParameterControl> HalftonePass::GetParameters()
{
	return {
		{ "Input", RenderPass::WidgetType::RENDER_TARGET, std::ref(m_inputs[0]) },
		{ "Is Monotone?", RenderPass::WidgetType::CHECKBOX, std::ref(m_halftoneBuffer.isMonotone) },
		{ "Dot Size", RenderPass::WidgetType::INT, std::ref(m_halftoneBuffer.dotSize) },
		{ "Dot Color", RenderPass::WidgetType::COLOR, std::ref(m_halftoneBuffer.dotColor) },
		{ "Channel Angle Offsets", RenderPass::WidgetType::FLOAT3, std::ref(m_halftoneBuffer.channelOffsets) }
	};
}

void HalftonePass::Update(ID3D11DeviceContext* deviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMMATRIX lightViewProj, XMVECTOR lightDirection, XMFLOAT3 clearColor, UINT width, UINT height)
{
	XMFLOAT2 offset = { m_halftoneBuffer.dotSize / static_cast<float>(width), m_halftoneBuffer.dotSize / static_cast<float>(height) };

	m_halftoneBuffer.subdivisions = offset;

	D3D11_MAPPED_SUBRESOURCE mapped = {};

	deviceContext->Map(m_constantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	std::memcpy(mapped.pData, &m_halftoneBuffer, sizeof(m_halftoneBuffer));
	deviceContext->Unmap(m_constantBuffers[0].Get(), 0);
}