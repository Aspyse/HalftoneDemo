#include "halftone_pass.h"

bool HalftonePass::InitializeConstantBuffer(ID3D11Device* device)
{
	ComPtr<ID3D11Buffer> halftoneBuffer;
	
	D3D11_BUFFER_DESC hbd;
	ZeroMemory(&hbd, sizeof(hbd));
	hbd.Usage = D3D11_USAGE_DYNAMIC;
	hbd.ByteWidth = sizeof(HalftoneBufferType);
	hbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hbd.MiscFlags = 0;
	hbd.StructureByteStride = 0;

	HRESULT result = device->CreateBuffer(&hbd, nullptr, halftoneBuffer.GetAddressOf());
	if (FAILED(result))
		return false;

	m_constantBuffers.push_back(halftoneBuffer);

	return true;
}

bool HalftonePass::SetShaderParameters(ID3D11DeviceContext* deviceContext, float dotSize, UINT texWidth, UINT texHeight)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = deviceContext->Map(m_constantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	HalftoneBufferType* dataPtr = (HalftoneBufferType*)mappedResource.pData;

	// TODO: fix
	dataPtr->dotSize = dotSize;

	deviceContext->Unmap(m_constantBuffers[0].Get(), 0);

	return true;
}