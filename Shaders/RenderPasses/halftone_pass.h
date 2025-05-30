#pragma once

#include "render_pass.h"
#include <DirectXMath.h>

using DirectX::XMMATRIX;
using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMVECTOR;

// De-facto template for Render Pass definitions
class HalftonePass : public RenderPass
{
private:
	struct HalftoneBufferType
	{
		float dotSize;
		float padding[3];
	};

public:
	bool SetShaderParameters(ID3D11DeviceContext*, float, UINT, UINT);

private:
	bool InitializeConstantBuffer(ID3D11Device*) override;
};