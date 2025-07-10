#include "render_pass.h"

RenderPass::RenderPass()
{
	m_outputRT = std::make_shared<RenderTarget>();
}

bool RenderPass::Initialize(ID3D11Device* device, UINT textureWidth, UINT textureHeight)
{
	m_outputRT->Initialize(device, textureWidth, textureHeight);
	
	wchar_t vsFilename[128];
	wchar_t psFilename[128];

	int error = wcscpy_s(psFilename, 128, filename());
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
	result = D3DCompileFromFile(psFilename, nullptr, nullptr, "PostprocessShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
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

	InitializeConstantBuffer(device);

	return true;
}

// IMMEDIATE TODO: refactor this part
bool RenderPass::Render(ID3D11DeviceContext* deviceContext, float* clearColor)
{
	if (Begin) Begin();

	for (UINT i = 0; i < m_constantBuffers.size(); ++i)
		deviceContext->PSSetConstantBuffers(i, 1, m_constantBuffers[i].GetAddressOf());

	m_inputRT->BindAsResource(deviceContext);

	m_outputRT->ClearTarget(deviceContext, clearColor);

	m_outputRT->BindAsTarget(deviceContext);

	RenderFrame(deviceContext);

	return true;
}

void RenderPass::WrapInput(ID3D11ShaderResourceView* const* resource, UINT numViews)
{
	m_inputRT = std::make_shared<RenderTarget>();
	m_inputRT->SetResource(resource, numViews);
}

void RenderPass::SetInput(std::shared_ptr<RenderTarget> input)
{
	m_inputRT = input;
}

void RenderPass::WrapOutput(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv)
{
	m_outputRT = std::make_shared<RenderTarget>();
	m_outputRT->SetTarget(rtv, dsv);
}

void RenderPass::SetOutput(std::shared_ptr<RenderTarget> output)
{
	m_outputRT = output;
}

std::shared_ptr<RenderTarget> RenderPass::GetOutput()
{
	return m_outputRT;
}

bool RenderPass::RenderFrame(ID3D11DeviceContext* deviceContext)
{
	deviceContext->IASetInputLayout(nullptr);

	deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	deviceContext->Draw(3, 0);

	return true;
}