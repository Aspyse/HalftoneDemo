#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
using namespace DirectX;
using namespace std;

class HalftoneShader
{
private:
	struct HalftoneBufferType
	{
		float dotSize;
		float padding[3];
	};

public:
	HalftoneShader();
	HalftoneShader(const HalftoneShader&);
	~HalftoneShader();

	bool Initialize(ID3D11Device*, const wchar_t*, LPCSTR);
	void Shutdown();
	bool SetShaderParameters(ID3D11DeviceContext*, float);
	bool Render(ID3D11DeviceContext*);

private:
	void OutputShaderErrorMessage(ID3D10Blob*, WCHAR*);

private:
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;

	ID3D11SamplerState* m_sampleStateWrap = nullptr;
	ID3D11Buffer* m_halftoneBuffer = nullptr;
};