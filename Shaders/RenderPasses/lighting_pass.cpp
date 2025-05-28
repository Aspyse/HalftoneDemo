#include "lighting_pass.h"

bool LightingPass::InitializeConstantBuffer(ID3D11Device* device)
{
	ComPtr<ID3D11Buffer> matrixBuffer, lightBuffer;
	
	D3D11_BUFFER_DESC mbd;
	ZeroMemory(&mbd, sizeof(mbd));
	mbd.Usage = D3D11_USAGE_DYNAMIC;
	mbd.ByteWidth = sizeof(MatrixBufferType);
	mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mbd.MiscFlags = 0;
	mbd.StructureByteStride = 0;

	HRESULT result = device->CreateBuffer(&mbd, nullptr, matrixBuffer.GetAddressOf());
	if (FAILED(result))
		return false;

	D3D11_BUFFER_DESC lbd;
	ZeroMemory(&lbd, sizeof(lbd));
	lbd.Usage = D3D11_USAGE_DYNAMIC;
	lbd.ByteWidth = sizeof(LightBufferType);
	lbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lbd.MiscFlags = 0;
	lbd.StructureByteStride = 0;

	result = device->CreateBuffer(&lbd, nullptr, lightBuffer.GetAddressOf());
	if (FAILED(result))
		return false;

	m_constantBuffers.push_back(matrixBuffer);
	m_constantBuffers.push_back(lightBuffer);

	return true;
}

bool LightingPass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX projectionMatrix, XMMATRIX viewMatrix, XMMATRIX lightViewProj, XMFLOAT3 lightDirectionVS, XMFLOAT3 lightColor, float* ambientColor, float ambientStrength, float celThreshold)
{
	XMMATRIX invProj = XMMatrixInverse(nullptr, projectionMatrix);
	XMMATRIX invView = XMMatrixInverse(nullptr, viewMatrix);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = deviceContext->Map(m_constantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

	dataPtr->invProj = XMMatrixTranspose(invProj);
	dataPtr->invView = XMMatrixTranspose(invView);

	deviceContext->Unmap(m_constantBuffers[0].Get(), 0);

	XMFLOAT3 ambientLight = XMFLOAT3(ambientColor[0] * ambientStrength, ambientColor[1] * ambientStrength, ambientColor[2] * ambientStrength);


	result = deviceContext->Map(m_constantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	LightBufferType* dataPtr2 = (LightBufferType*)mappedResource.pData;

	dataPtr2->lightViewProj = XMMatrixTranspose(lightViewProj);
	dataPtr2->lightDirectionVS = lightDirectionVS;
	dataPtr2->celThreshold = celThreshold;
	dataPtr2->lightColor = lightColor;
	dataPtr2->ambientColor = ambientLight;

	deviceContext->Unmap(m_constantBuffers[1].Get(), 0);

	return true;
}