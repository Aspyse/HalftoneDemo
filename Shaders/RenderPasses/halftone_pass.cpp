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
		{ "Is Monotone?", RenderPass::WidgetType::CHECKBOX, std::ref(m_halftoneBuffer.isMonotone) }
	};
}

bool HalftonePass::SetShaderParameters(ID3D11DeviceContext* deviceContext, float* inkColor, UINT width, UINT height, bool isMonotone, float* channelOffsets)
{
	XMFLOAT2 subdivisions = XMFLOAT2(1 / static_cast<float>(width), 1 / static_cast<float>(height));
	XMFLOAT3 inkColorX = XMFLOAT3(inkColor[0], inkColor[1], inkColor[2]);
	XMFLOAT3 channelOffsetsX = XMFLOAT3(channelOffsets[0], channelOffsets[1], channelOffsets[2]);
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = deviceContext->Map(m_constantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	HalftoneBufferType* dataPtr = (HalftoneBufferType*)mappedResource.pData;

	// TODO: fix
	dataPtr->isMonotone = isMonotone;
	dataPtr->subdivisions = subdivisions;
	dataPtr->dotColor = inkColorX;
	dataPtr->channelOffsets = channelOffsetsX;

	deviceContext->Unmap(m_constantBuffers[0].Get(), 0);

	return true;
}