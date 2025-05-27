#include "crosshatch_pass.h"

bool CrosshatchPass::InitializeConstantBuffer(ID3D11Device* device)
{
	ComPtr<ID3D11Buffer> sobelBuffer;

	D3D11_BUFFER_DESC sbd;
	ZeroMemory(&sbd, sizeof(sbd));
	sbd.Usage = D3D11_USAGE_DYNAMIC;
	sbd.ByteWidth = sizeof(CrosshatchBufferType);
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

bool CrosshatchPass::SetShaderParameters(ID3D11DeviceContext* deviceContext, UINT width, UINT height, float thicknessMul, float topoFreqMul, float radFreqMul)
{
	XMFLOAT2 offset = XMFLOAT2(1 / static_cast<float>(width), 1 / static_cast<float>(height));

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = deviceContext->Map(m_constantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	CrosshatchBufferType* dataPtr = (CrosshatchBufferType*)mappedResource.pData;

	dataPtr->offset = offset;
	dataPtr->thicknessMul = thicknessMul;
	dataPtr->topoFreqMul = topoFreqMul;
	dataPtr->radFreqMul = radFreqMul;

	deviceContext->Unmap(m_constantBuffers[0].Get(), 0);

	return true;
}