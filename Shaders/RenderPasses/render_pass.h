#pragma once

#include "render_target.h"
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
	RenderPass();

	std::function<void()> Begin; // Lambda for processing before rendering, e.g. binding extra samplers

	bool Initialize(ID3D11Device*, UINT, UINT);

	bool Render(ID3D11DeviceContext*, float*);

	void WrapInput(ID3D11ShaderResourceView* const*, UINT);
	void SetInput(std::shared_ptr<RenderTarget>);

	void WrapOutput(ID3D11RenderTargetView*, ID3D11DepthStencilView*);
	void SetOutput(std::shared_ptr<RenderTarget>);

	std::shared_ptr<RenderTarget> GetOutput();

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

protected:
	virtual const wchar_t* filename() const = 0;

private:
	std::shared_ptr<RenderTarget> m_inputRT;
	std::shared_ptr<RenderTarget> m_outputRT;

	UINT m_numTargetViews = 0, m_numResourceViews = 0;

	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;

protected:
	std::vector<ComPtr<ID3D11Buffer>> m_constantBuffers;
};
