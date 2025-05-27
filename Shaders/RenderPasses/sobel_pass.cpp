#include "sobel_pass.h"

bool SobelPass::InitializeConstantBuffer(ID3D11Device* device)
{
	ComPtr<ID3D11Buffer> sobelBuffer;

	D3D11_BUFFER_DESC sbd;
	ZeroMemory(&sbd, sizeof(sbd));
	sbd.Usage = D3D11_USAGE_DYNAMIC;
	sbd.ByteWidth = sizeof(SobelBufferType);
	sbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	sbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	sbd.MiscFlags = 0;
	sbd.StructureByteStride = 0;

	HRESULT result = device->CreateBuffer(&sbd, nullptr, sobelBuffer.GetAddressOf());
	if (FAILED(result))
		return false;

	m_constantBuffers.push_back(sobelBuffer);

	return true;
}

bool SobelPass::SetShaderParameters(ID3D11DeviceContext* deviceContext, UINT width, UINT height, bool isScharr, float threshold, float* inkColor)
{
	XMFLOAT2 offset = XMFLOAT2(1 / static_cast<float>(width), 1 / static_cast<float>(height));
	threshold *= threshold; // artist control
	XMFLOAT3 inkColorX = XMFLOAT3(inkColor[0], inkColor[1], inkColor[2]);
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = deviceContext->Map(m_constantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	SobelBufferType* dataPtr = (SobelBufferType*)mappedResource.pData;

	dataPtr->isScharr = isScharr;
	dataPtr->offset = offset;
	dataPtr->threshold = threshold;
	dataPtr->inkColor = inkColorX;

	deviceContext->Unmap(m_constantBuffers[0].Get(), 0);

	return true;
}