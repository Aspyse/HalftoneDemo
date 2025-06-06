#include "render_pass.h"

bool RenderPass::Initialize(ID3D11Device* device, const wchar_t* pixelFilename)
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

bool RenderPass::Render(ID3D11DeviceContext* deviceContext, float* clearColor)
{
	if (Begin) Begin();

	for (UINT i = 0; i < m_constantBuffers.size(); ++i)
		deviceContext->PSSetConstantBuffers(i, 1, m_constantBuffers[i].GetAddressOf());

	deviceContext->PSSetShaderResources(0, m_numResourceViews, m_shaderResource);

	deviceContext->ClearRenderTargetView(m_renderTarget, clearColor);
	if (m_dsv)
		deviceContext->ClearDepthStencilView(m_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	deviceContext->OMSetRenderTargets(m_numTargetViews, &m_renderTarget, m_dsv);

	RenderFrame(deviceContext);

	return true;
}

void RenderPass::AssignShaderResource(ID3D11ShaderResourceView* const* resource, UINT numViews)
{
	m_shaderResource = resource;
	m_numResourceViews = numViews;
}

void RenderPass::AssignRenderTarget(ID3D11RenderTargetView* target, UINT numViews, ID3D11DepthStencilView* pDepthStencilView)
{
	m_renderTarget = target;
	m_numTargetViews = numViews;
	m_dsv = pDepthStencilView;
}

bool RenderPass::RenderFrame(ID3D11DeviceContext* deviceContext)
{
	deviceContext->IASetInputLayout(nullptr);

	deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	deviceContext->Draw(3, 0);

	return true;
}