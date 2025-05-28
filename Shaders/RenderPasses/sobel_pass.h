#pragma once

#include "render_pass.h"
#include <DirectXMath.h>

using DirectX::XMMATRIX;
using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMVECTOR;

// De-facto template for Render Pass definitions
class SobelPass : public RenderPass
{
private:
	struct SobelBufferType
	{
		int isScharr;
		XMFLOAT2 offset;
		float threshold;

		XMFLOAT3 inkColor;
		float pad0;

		XMFLOAT3 clearColor;
		float pad1;
	};

public:
	bool SetShaderParameters(ID3D11DeviceContext*, UINT, UINT, bool, float, float*, float*);

private:
	bool InitializeConstantBuffer(ID3D11Device*) override;
};