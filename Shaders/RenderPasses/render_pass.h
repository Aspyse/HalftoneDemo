#pragma once

#include <fstream>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <functional> // TODO: check if expensive overhead
#include <vector>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
using std::ofstream;

class RenderPass
{
public:
	std::function<void()> Begin; // Lambda for processing before rendering, e.g. binding extra samplers

	bool Initialize(ID3D11Device* device, const wchar_t* pixelFilename)
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

	bool Render(ID3D11DeviceContext* deviceContext, float* clearColor)
	{
		if (Begin) Begin();

		for (UINT i = 0; i < m_constantBuffers.size(); ++i)
			deviceContext->PSSetConstantBuffers(i, 1, m_constantBuffers[i].GetAddressOf());

		deviceContext->PSSetShaderResources(0, m_numResourceViews, m_shaderResource); // check if working

		deviceContext->ClearRenderTargetView(m_renderTarget, clearColor);
		if (m_dsv)
			deviceContext->ClearDepthStencilView(m_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

		deviceContext->OMSetRenderTargets(m_numTargetViews, &m_renderTarget, m_dsv);

		RenderFrame(deviceContext);

		return true;
	}

	void AssignShaderResource(ID3D11ShaderResourceView** resource, UINT numViews)
	{
		m_shaderResource = resource;
		m_numResourceViews = numViews;
	}

	void AssignRenderTarget(ID3D11RenderTargetView* target, UINT numViews, ID3D11DepthStencilView* pDepthStencilView)
	{
		m_renderTarget = target;
		m_numTargetViews = numViews;
		m_dsv = pDepthStencilView;
	}

private:
	bool RenderFrame(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->IASetInputLayout(nullptr);

		deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
		deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

		deviceContext->Draw(3, 0);

		return true;
	}

	virtual bool InitializeConstantBuffer(ID3D11Device* device) = 0;



	static void OutputShaderErrorMessage(ID3D10Blob* errorMessage, WCHAR* shaderFilename)
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

private:
	ID3D11RenderTargetView* m_renderTarget;
	ID3D11ShaderResourceView** m_shaderResource = nullptr;
	ID3D11DepthStencilView* m_dsv = nullptr;

	UINT m_numTargetViews = 0, m_numResourceViews = 0;

	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;

	// list of UI parameters

protected:
	std::vector<ComPtr<ID3D11Buffer>> m_constantBuffers;
};
