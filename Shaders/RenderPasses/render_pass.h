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

	bool Initialize(ID3D11Device*, const wchar_t*);

	bool Render(ID3D11DeviceContext*, float*);

	void AssignShaderResource(ID3D11ShaderResourceView* const*, UINT);

	void AssignRenderTarget(ID3D11RenderTargetView*, UINT, ID3D11DepthStencilView*);

private:
	bool RenderFrame(ID3D11DeviceContext*);

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
	ID3D11RenderTargetView* m_renderTarget = nullptr;
	ID3D11ShaderResourceView* const* m_shaderResource = nullptr;
	ID3D11DepthStencilView* m_dsv = nullptr;

	UINT m_numTargetViews = 0, m_numResourceViews = 0;

	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;

protected:
	std::vector<ComPtr<ID3D11Buffer>> m_constantBuffers;
};
