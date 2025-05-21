#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
using namespace DirectX;
using namespace std;

// Base class for lighting pass shaders
class LightingShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX invProj;
		XMMATRIX invView;
	};
	struct LightBufferType
	{
		XMMATRIX lightViewProj;
		XMFLOAT3 lightDirectionVS;
		float pad0;
		XMFLOAT3 lightColor;
		float pad1;
		XMFLOAT3 ambientColor;
		float pad2;
	};

public:
	LightingShader();
	LightingShader(const LightingShader&);
	~LightingShader();

	bool Initialize(ID3D11Device*, const wchar_t*);
	void Shutdown();
	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, XMVECTOR, XMFLOAT3, XMFLOAT3, float);
	bool Render(ID3D11DeviceContext*);

private:
	bool CompileShader(ID3D11Device*, WCHAR*);
	void OutputShaderErrorMessage(ID3D10Blob*, WCHAR*);

	bool InitializeSampler(ID3D11Device*);

private:
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;

	ID3D11Buffer* m_matrixBuffer = nullptr;
	ID3D11Buffer* m_lightBuffer = nullptr;

	ID3D11SamplerState* m_sampleStateWrap = nullptr;
	ID3D11SamplerState* m_shadowSampler = nullptr;
};