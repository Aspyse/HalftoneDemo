#pragma once

#include "render_pass.h"
#include <DirectXMath.h>

using DirectX::XMMATRIX;
using DirectX::XMFLOAT3;
using DirectX::XMVECTOR;

class HalftonePass : public RenderPass
{
private:
	struct HalftoneBufferType
	{
		float dotSize;
		float padding[3];
	};

public:
	bool SetShaderParameters(ID3D11DeviceContext*, float);

private:
	bool InitializeConstantBuffer(ID3D11Device* device) override;
};