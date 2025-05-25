#include "lighting_shader.h"

LightingShader::LightingShader() {}
LightingShader::LightingShader(const LightingShader&) {}
LightingShader::~LightingShader() {}

using namespace std;

bool LightingShader::Initialize(ID3D11Device* device, const wchar_t* pixelFilename)
{
	wchar_t vsFilename[128];
	wchar_t psFilename[128];

	int error = wcscpy_s(psFilename, 128, pixelFilename);
	if (error != 0)
		return false;

	if (!CompileShader(device, psFilename))
		return false;

	if (!InitializeSampler(device))
		return false;

	return true;
}

bool LightingShader::Render(ID3D11DeviceContext* deviceContext) // Consider splitting
{
	deviceContext->PSSetSamplers(0, 1, &m_sampleStateWrap);
	deviceContext->PSSetSamplers(1, 1, &m_shadowSampler);
	
	deviceContext->IASetInputLayout(nullptr);

	deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	deviceContext->Draw(3, 0);
	return true;
}

bool LightingShader::CompileShader(ID3D11Device* device, WCHAR* psFilename)
{
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;

	wchar_t vsFilename[128];
	int error = wcscpy_s(vsFilename, 128, L"Shaders/base.vs");
	if (error != 0)
		return false;

	// Compile vertex shader code
	HRESULT result = D3DCompileFromFile(vsFilename, nullptr, nullptr, "BaseVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, vsFilename);
		return false;
	}
	
	// Compile pixel shader code
	result = D3DCompileFromFile(psFilename, nullptr, nullptr, "LightPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, psFilename);
		return false;
	}

	// Create vertex shader from buffer
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(result))
		return false;

	// Create pixel shader from buffer
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(result))
		return false;

	// Release vertex and pixel shader buffers
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// Create dynamic matrix constant buffer description
	D3D11_BUFFER_DESC mbd;
	ZeroMemory(&mbd, sizeof(mbd));
	mbd.Usage = D3D11_USAGE_DYNAMIC;
	mbd.ByteWidth = sizeof(MatrixBufferType);
	mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mbd.MiscFlags = 0;
	mbd.StructureByteStride = 0;

	result = device->CreateBuffer(&mbd, nullptr, &m_matrixBuffer);
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

	result = device->CreateBuffer(&lbd, nullptr, &m_lightBuffer);
	if (FAILED(result))
		return false;

	return true;
}

bool LightingShader::InitializeSampler(ID3D11Device* device)
{
	HRESULT result;

	// Regular sampler (pointClamp)
	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.MipLODBias = 0.0f;
	sd.MaxAnisotropy = 1;
	sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sd.BorderColor[0] = 0;
	sd.BorderColor[1] = 0;
	sd.BorderColor[2] = 0;
	sd.BorderColor[3] = 0;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	result = device->CreateSamplerState(&sd, &m_sampleStateWrap);
	if (FAILED(result))
		return false;

	// Shadow comparison sampler (shadowSampler)
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

	result = device->CreateSamplerState(&shadowDesc, &m_shadowSampler);
	if (FAILED(result))
		return false;

	return true;
}

void LightingShader::Shutdown() // Consider splitting up
{
	if (m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = nullptr;
	}
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = nullptr;
	}
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = nullptr;
	}
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = nullptr;
	}
}

bool LightingShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX projectionMatrix, XMMATRIX viewMatrix, XMMATRIX lightViewProj, XMVECTOR lightDirection, XMFLOAT3 lightColor, XMFLOAT3 ambientColor, float celThreshold)
{
	XMMATRIX invProj = XMMatrixInverse(nullptr, projectionMatrix);
	XMMATRIX invView = XMMatrixInverse(nullptr, viewMatrix);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

	dataPtr->invProj = XMMatrixTranspose(invProj);
	dataPtr->invView = XMMatrixTranspose(invView);

	deviceContext->Unmap(m_matrixBuffer, 0);

	UINT bufferNumber = 0;

	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);



	// Update shader parameters
	XMVECTOR lightDirectionVecVS = XMVector3TransformNormal(lightDirection, viewMatrix);
	XMFLOAT3 lightDirectionVS;
	XMStoreFloat3(&lightDirectionVS, lightDirectionVecVS);

	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	LightBufferType* dataPtr2 = (LightBufferType*)mappedResource.pData;

	dataPtr2->lightViewProj = XMMatrixTranspose(lightViewProj);
	dataPtr2->lightDirectionVS = lightDirectionVS;
	dataPtr2->lightColor = lightColor;
	dataPtr2->ambientColor = ambientColor;

	deviceContext->Unmap(m_lightBuffer, 0);

	bufferNumber = 1;

	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

	return true;
}

void LightingShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long long bufferSize, i;
	ofstream fout;

	compileErrors = (char*)(errorMessage->GetBufferPointer());

	bufferSize = errorMessage->GetBufferSize();
	fout.open("shader-error.txt");
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}
	fout.close();

	errorMessage->Release();
	errorMessage = 0;
}