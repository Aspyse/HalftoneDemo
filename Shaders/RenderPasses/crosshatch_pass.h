#pragma once

#include "render_pass.h"
#include <DirectXMath.h>

using DirectX::XMFLOAT2;

class CrosshatchPass : public RenderPass
{
private:
	struct CrosshatchBufferType
	{
		XMFLOAT2 offset;
		float thicknessMul;
		float topoFreqMul;
		float radFreqMul;

		float pad[3];
	};

public:
	bool SetShaderParameters(ID3D11DeviceContext*, UINT, UINT, float, float, float);

private:
	bool InitializeConstantBuffer(ID3D11Device*) override;
};