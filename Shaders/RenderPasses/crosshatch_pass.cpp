#include "crosshatch_pass.h"

bool CrosshatchPass::InitializeConstantBuffer(ID3D11Device* device)
{
	ComPtr<ID3D11Buffer> crosshatchBuffer;

	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(CrosshatchBufferType);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0;
	cbd.StructureByteStride = 0;

	HRESULT result = device->CreateBuffer(&cbd, nullptr, crosshatchBuffer.GetAddressOf());
	if (FAILED(result))
		return false;

	m_constantBuffers.push_back(crosshatchBuffer);

	return true;
}

bool CrosshatchPass::SetShaderParameters(ID3D11DeviceContext* deviceContext, float thicknessMul, float topoFreqMul, XMFLOAT3 lightDirectionVS, float* inkColor, float thresholdA, float thresholdB, float* clearColor, float hatchAngle, bool isFeather)
{
	XMFLOAT3 inkColorX = XMFLOAT3(inkColor[0], inkColor[1], inkColor[2]);
	XMFLOAT3 clearColorX = XMFLOAT3(clearColor[0], clearColor[1], clearColor[2]);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = deviceContext->Map(m_constantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	CrosshatchBufferType* dataPtr = (CrosshatchBufferType*)mappedResource.pData;

	dataPtr->thicknessMul = thicknessMul;
	dataPtr->topoFreqMul = topoFreqMul;
	dataPtr->thresholdA = thresholdA;
	dataPtr->thresholdB = thresholdB;

	dataPtr->lightDirectionVS = lightDirectionVS;
	dataPtr->inkColor = inkColorX;
	dataPtr->clearColor = clearColorX;
	dataPtr->hatchAngle = hatchAngle;
	dataPtr->isFeather = isFeather;

	deviceContext->Unmap(m_constantBuffers[0].Get(), 0);

	return true;
}