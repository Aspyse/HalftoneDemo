#include "pioaru_pass.h"

bool PioaruPass::InitializeConstantBuffer(ID3D11Device* device)
{
	ComPtr<ID3D11Buffer> pioaruBuffer;

	D3D11_BUFFER_DESC pbd;
	ZeroMemory(&pbd, sizeof(pbd));
	pbd.Usage = D3D11_USAGE_DYNAMIC;
	pbd.ByteWidth = sizeof(PioaruBufferType);
	pbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pbd.MiscFlags = 0;
	pbd.StructureByteStride = 0;

	HRESULT result = device->CreateBuffer(&pbd, nullptr, pioaruBuffer.GetAddressOf());
	if (FAILED(result))
		return false;

	m_constantBuffers.push_back(pioaruBuffer);

	return true;
}

bool PioaruPass::SetShaderParameters(ID3D11DeviceContext* deviceContext, float* inkColor, XMFLOAT3 lightDirectionVS, UINT width, UINT height)
{
	XMFLOAT2 subdivisions = XMFLOAT2(1 / static_cast<float>(width), 1 / static_cast<float>(height));
	XMFLOAT3 inkColorX = XMFLOAT3(inkColor[0], inkColor[1], inkColor[2]);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = deviceContext->Map(m_constantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	PioaruBufferType* dataPtr = (PioaruBufferType*)mappedResource.pData;

	dataPtr->subdivisions = subdivisions;
	dataPtr->inkColor = inkColorX;
	dataPtr->lightDirVS = lightDirectionVS;

	deviceContext->Unmap(m_constantBuffers[0].Get(), 0);

	return true;
}