#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;

class GeometryPass
{
private:
	struct ShadowBufferType
	{
		XMMATRIX lightViewProj;
	};
	struct CameraBufferType
	{
		XMMATRIX worldMatrix;
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
	};
	struct MaterialBufferType
	{
		XMFLOAT3 albedoColor;
		float roughness;

		int useAlbedoTexture;
		int useNormalTexture;
		int useRoughnessTexture;
		
		float padding;

		XMMATRIX viewMatrix;
	};

public:
	GeometryPass();
	GeometryPass(const GeometryPass&);
	~GeometryPass();

	bool Initialize(ID3D11Device*, UINT, UINT);
	bool InitializeGBuffer(ID3D11Device*, UINT, UINT);
	void Shutdown();

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, XMFLOAT3, float, bool, bool, bool); // TODO: take in texture

	void ClearGBuffer(ID3D11DeviceContext*, float*);
	void Render(ID3D11DeviceContext*, int);
	bool RenderShadow(ID3D11DeviceContext*, int, XMVECTOR);

	ID3D11ShaderResourceView* GetGBuffer(UINT);
	ID3D11ShaderResourceView* GetShadowMap();
	XMMATRIX GetLightViewProj() const;

private:
	bool CompileShader(ID3D11Device*);
	bool InitializeSampler(ID3D11Device*);
	bool InitializeShadow(ID3D11Device*);

	void OutputShaderErrorMessage(ID3D10Blob*, WCHAR*);

private:
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout *m_layout = nullptr, * m_shadowLayout = nullptr;
	ID3D11SamplerState* m_sampleStateWrap = nullptr;
	ID3D11Buffer *m_cameraBuffer = nullptr, *m_materialBuffer = nullptr, *m_shadowBuffer = nullptr;

	ID3D11RenderTargetView *m_albedoRTV = nullptr, *m_normalRTV = nullptr;
	ID3D11ShaderResourceView *m_albedoSRV = nullptr, *m_normalSRV = nullptr;

	ID3D11ShaderResourceView* m_depthSRV = nullptr;
	ID3D11DepthStencilView* m_dsv = nullptr;

	ID3D11VertexShader* m_shadowShader = nullptr;
	ID3D11DepthStencilView* m_shadowDSV = nullptr;
	ID3D11ShaderResourceView* m_shadowSRV = nullptr;

	UINT m_shadowMapSize = 4096;
	D3D11_VIEWPORT m_shadowVp = {};
	XMMATRIX m_lightViewProj = {};
};