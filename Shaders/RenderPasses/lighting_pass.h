#pragma once

#include "render_pass.h"
#include <DirectXMath.h>

using DirectX::XMMATRIX;
using DirectX::XMFLOAT3;

class LightingPass : public RenderPass
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
		float celThreshold;
		XMFLOAT3 lightColor;
		float pad1;
		XMFLOAT3 ambientColor;
		float pad2;
	};

public:
	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, XMFLOAT3, XMFLOAT3, float*, float, float);

private:
	bool InitializeConstantBuffer(ID3D11Device*) override;
};