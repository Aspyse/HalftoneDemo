#include "canny_pass.h"

bool CannyPass::Initialize(ID3D11Device* device, UINT textureWidth, UINT textureHeight)
{
	wchar_t vsFilename[128];
	wchar_t psFilename1[128];
	wchar_t psFilename2[128];
	wchar_t psFilename3[128];

	wchar_t gaussianFilename[128];

	int error = wcscpy_s(vsFilename, 128, L"Shaders/base.vs");
	if (error != 0)
		return false;

	error = wcscpy_s(psFilename1, 128, L"Shaders/sobel.ps");
	if (error != 0)
		return false;

	error = wcscpy_s(psFilename2, 128, L"Shaders/nms.ps");
	if (error != 0)
		return false;
	
	error = wcscpy_s(psFilename3, 128, L"Shaders/dilate.ps");
	if (error != 0)
		return false;

	error = wcscpy_s(gaussianFilename, 128, L"Shaders/gaussian.ps");
	if (error != 0)
		return false;

	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	// Compile vertex shader code
	HRESULT result = D3DCompileFromFile(vsFilename, nullptr, nullptr, "BaseVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, vsFilename);
		return false;
	}
	// Create vertex shader from buffer
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(result))
		return false;

	ID3D10Blob* pixelShaderBuffer1;
	ID3D10Blob* pixelShaderBuffer2;
	ID3D10Blob* pixelShaderBuffer3;
	// Compile pixel shader code
	result = D3DCompileFromFile(psFilename1, nullptr, nullptr, "PostprocessShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer1, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, psFilename1);
		return false;
	}

	result = D3DCompileFromFile(psFilename2, nullptr, nullptr, "PostprocessShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer2, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, psFilename2);
		return false;
	}

	result = D3DCompileFromFile(psFilename3, nullptr, nullptr, "PostprocessShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer3, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, psFilename3);
		return false;
	}

	ID3D10Blob* gaussianShaderBuffer1;
	ID3D10Blob* gaussianShaderBuffer2;

	result = D3DCompileFromFile(gaussianFilename, nullptr, nullptr, "GaussianH", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &gaussianShaderBuffer1, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, gaussianFilename);
		return false;
	}

	result = D3DCompileFromFile(gaussianFilename, nullptr, nullptr, "GaussianV", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &gaussianShaderBuffer2, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, gaussianFilename);
		return false;
	}


	// Create pixel shader from buffer
	result = device->CreatePixelShader(pixelShaderBuffer1->GetBufferPointer(), pixelShaderBuffer1->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(result))
		return false;

	result = device->CreatePixelShader(pixelShaderBuffer2->GetBufferPointer(), pixelShaderBuffer2->GetBufferSize(), nullptr, &m_pixelShader2);
	if (FAILED(result))
		return false;

	result = device->CreatePixelShader(pixelShaderBuffer3->GetBufferPointer(), pixelShaderBuffer3->GetBufferSize(), nullptr, &m_pixelShader3);
	if (FAILED(result))
		return false;

	result = device->CreatePixelShader(gaussianShaderBuffer1->GetBufferPointer(), gaussianShaderBuffer1->GetBufferSize(), nullptr, &m_gaussianShader1);
	if (FAILED(result))
		return false;

	result = device->CreatePixelShader(gaussianShaderBuffer2->GetBufferPointer(), gaussianShaderBuffer2->GetBufferSize(), nullptr, &m_gaussianShader2);
	if (FAILED(result))
		return false;


	// Release vertex and pixel shader buffers
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer1->Release();
	pixelShaderBuffer1 = 0;

	pixelShaderBuffer2->Release();
	pixelShaderBuffer2 = 0;

	pixelShaderBuffer3->Release();
	pixelShaderBuffer3 = 0;

	gaussianShaderBuffer1->Release();
	gaussianShaderBuffer1 = 0;

	gaussianShaderBuffer2->Release();
	gaussianShaderBuffer2 = 0;

	InitializeConstantBuffer(device);

	m_pingTarget = std::make_unique<RenderTarget>();
	m_pingTarget->Initialize(device, textureWidth, textureHeight);

	m_pongTarget = std::make_unique<RenderTarget>();
	m_pongTarget->Initialize(device, textureWidth, textureHeight);

	return true;
}

std::vector<RenderPass::ParameterControl> CannyPass::GetParameters()
{
	return {
		{ "Input", RenderPass::WidgetType::RENDER_TARGET, std::ref(m_inputs[0]) },
		{ "Gaussian Radius", RenderPass::WidgetType::INT, std::ref(m_gaussianBuffer.kernelRadius) },
		{ "Non-maximum Suppression", RenderPass::WidgetType::CHECKBOX, std::ref(m_nmsBuffer.isNMS) },
		{ "Is Depth", RenderPass::WidgetType::CHECKBOX, std::ref(m_gaussianBuffer.isDepth) },
		{ "Is Single Channel", RenderPass::WidgetType::CHECKBOX, std::ref(m_sobelBuffer.isMono) },
		{ "Stroke Color", RenderPass::WidgetType::COLOR, std::ref(m_dilateBuffer.inkColor) },
		{ "Stroke Threshold", RenderPass::WidgetType::FLOAT, std::ref(m_dilateBuffer.threshold) },
	};
}

void CannyPass::Update(ID3D11DeviceContext* deviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMMATRIX lightViewProj, XMVECTOR lightDirection, XMFLOAT3 clearColor, UINT width, UINT height)
{
	int halfSize = m_gaussianBuffer.kernelRadius;
	//float sigma = 0.3 * (halfSize - 1) + 0.8;
	float sigma = 1.0f;
	float t = sigma * sigma;                        // scale‐space parameter
	
	float sum = 0.0f;

	// Compute unnormalized weights
	for (int i = 0; i <= halfSize; ++i) {
		float In = std::cyl_bessel_i(i, t);
		float w = std::exp(-t) * In;
		m_gaussianBuffer.weights[i] = w;
		sum += (i == 0 ? w : 2 * w);
	}

	for (auto& w : m_gaussianBuffer.weights) {
		w /= sum;
	}


	XMFLOAT2 offset = {1/ static_cast<float>(width), 1/ static_cast<float>(height)};
	
	m_gaussianBuffer.texelSize = offset;
	m_gaussianBuffer.proj33 = projectionMatrix.r[2].m128_f32[2]; // _33
	m_gaussianBuffer.proj43 = projectionMatrix.r[3].m128_f32[2]; // _43

	m_sobelBuffer.offset = offset;
	m_nmsBuffer.offset = offset;
	m_dilateBuffer.offset = offset;
	
	D3D11_MAPPED_SUBRESOURCE mapped = {};


	deviceContext->Map(m_constantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	std::memcpy(mapped.pData, &m_sobelBuffer, sizeof(m_sobelBuffer));
	deviceContext->Unmap(m_constantBuffers[0].Get(), 0);

	deviceContext->Map(m_constantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	std::memcpy(mapped.pData, &m_nmsBuffer, sizeof(m_nmsBuffer));
	deviceContext->Unmap(m_constantBuffers[1].Get(), 0);

	deviceContext->Map(m_constantBuffers[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	std::memcpy(mapped.pData, &m_dilateBuffer, sizeof(m_dilateBuffer));
	deviceContext->Unmap(m_constantBuffers[2].Get(), 0);

	deviceContext->Map(m_constantBuffers[3].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	std::memcpy(mapped.pData, &m_gaussianBuffer, sizeof(m_gaussianBuffer));
	deviceContext->Unmap(m_constantBuffers[3].Get(), 0);
}

// IMMEDIATE TODO: refactor this part
bool CannyPass::Render(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float* clearColor)
{
	// Store original target
	ID3D11RenderTargetView* tempTarget;
	deviceContext->OMGetRenderTargets(1, &tempTarget, nullptr);

	deviceContext->IASetInputLayout(nullptr);

	deviceContext->VSSetShader(m_vertexShader, nullptr, 0);

	ID3D11RenderTargetView* nullRTV = nullptr;
	
	// Gaussian H
	deviceContext->OMSetRenderTargets(1, m_pingTarget->GetTarget(), nullptr);

	deviceContext->PSSetConstantBuffers(0, 1, m_constantBuffers[3].GetAddressOf());

	deviceContext->PSSetShader(m_gaussianShader1, nullptr, 0);

	deviceContext->Draw(3, 0);

	deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

	// Gaussian V
	deviceContext->PSSetShaderResources(0, 1, m_pingTarget->GetResourceView());
	deviceContext->OMSetRenderTargets(1, m_pongTarget->GetTarget(), nullptr);
	//deviceContext->OMSetRenderTargets(1, &tempTarget, nullptr);

	deviceContext->PSSetShader(m_gaussianShader2, nullptr, 0);

	deviceContext->Draw(3, 0);

	deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
	//return true;

	// Sobel phase
	deviceContext->PSSetShaderResources(0, 1, m_pongTarget->GetResourceView());
	deviceContext->OMSetRenderTargets(1, m_pingTarget->GetTarget(), nullptr);

	deviceContext->PSSetConstantBuffers(0, 1, m_constantBuffers[0].GetAddressOf());

	deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	deviceContext->Draw(3, 0);

	deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

	// NMR phase
	deviceContext->PSSetShaderResources(0, 1, m_pingTarget->GetResourceView());
	deviceContext->OMSetRenderTargets(1, m_pongTarget->GetTarget(), nullptr);

	deviceContext->PSSetConstantBuffers(0, 1, m_constantBuffers[1].GetAddressOf());

	deviceContext->PSSetShader(m_pixelShader2, nullptr, 0);

	deviceContext->Draw(3, 0);

	deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

	// Dilate phase
	deviceContext->PSSetShaderResources(0, 1, m_pongTarget->GetResourceView());
	deviceContext->OMSetRenderTargets(1, &tempTarget, nullptr);

	deviceContext->PSSetConstantBuffers(0, 1, m_constantBuffers[2].GetAddressOf());

	deviceContext->PSSetShader(m_pixelShader3, nullptr, 0);

	deviceContext->Draw(3, 0);

	return true;
}

bool CannyPass::InitializeConstantBuffer(ID3D11Device* device)
{
	AddCB<SobelBufferType>(device);
	AddCB<NMSBufferType>(device);
	AddCB<DilateBufferType>(device);
	AddCB<GaussianBufferType>(device);

	return true;
}