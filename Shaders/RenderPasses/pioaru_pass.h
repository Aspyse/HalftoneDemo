#pragma once

#include "render_pass.h"
#include <DirectXMath.h>

using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMVECTOR;

class PioaruPass : public RenderPass
{
private:
	struct PioaruBufferType
	{
		XMFLOAT3 inkColor;
		float pad0;

		XMFLOAT3 lightDirVS;
		float pad1;

		XMFLOAT2 subdivisions;

		float padding[2];
	};

public:
	bool SetShaderParameters(ID3D11DeviceContext*, float*, XMFLOAT3, UINT, UINT);

private:
	bool InitializeConstantBuffer(ID3D11Device*) override;
};