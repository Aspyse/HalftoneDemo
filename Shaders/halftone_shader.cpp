// TODO: CONVERT INTO GENERIC POSTPROCESS SHADER CLASS

#include "halftone_shader.h"

HalftoneShader::HalftoneShader() {}

bool HalftoneShader::Initialize(ID3D11Device* device, const wchar_t* pixelFilename, LPCSTR shaderName)
{
	wchar_t vsFilename[128];
	wchar_t psFilename[128];

	int error = wcscpy_s(psFilename, 128, pixelFilename);
	if (error != 0)
		return false;

	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;

	error = wcscpy_s(vsFilename, 128, L"Shaders/base.vs");
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
	result = D3DCompileFromFile(psFilename, nullptr, nullptr, shaderName, "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
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
	D3D11_BUFFER_DESC hbd;
	ZeroMemory(&hbd, sizeof(hbd));
	hbd.Usage = D3D11_USAGE_DYNAMIC;
	hbd.ByteWidth = sizeof(HalftoneBufferType);
	hbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hbd.MiscFlags = 0;
	hbd.StructureByteStride = 0;

	result = device->CreateBuffer(&hbd, nullptr, &m_halftoneBuffer);
	if (FAILED(result))
		return false;

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

	return true;
}

bool HalftoneShader::Render(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetSamplers(0, 1, &m_sampleStateWrap);

	deviceContext->IASetInputLayout(nullptr);

	deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	deviceContext->Draw(3, 0);
	return true;
}

bool HalftoneShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, float dotSize)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = deviceContext->Map(m_halftoneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	HalftoneBufferType* dataPtr = (HalftoneBufferType*)mappedResource.pData;

	dataPtr->dotSize = dotSize;

	deviceContext->Unmap(m_halftoneBuffer, 0);

	UINT bufferNumber = 0;

	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_halftoneBuffer);

	return true;
}

void HalftoneShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, WCHAR* shaderFilename)
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