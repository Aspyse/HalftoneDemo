#pragma once

#include "render_pass.h"
#include <DirectXMath.h>

using DirectX::XMMATRIX;
using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMVECTOR;

class BlendPass : public RenderPass
{
private:
	bool InitializeConstantBuffer(ID3D11Device*) override { return true; };
};