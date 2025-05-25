#pragma once
#include "DirectXMath.h"

using DirectX::XMMATRIX;
using DirectX::XMFLOAT3;

struct LightingBufferType
{
	// MatrixBuffer
	XMMATRIX invProj;
	XMMATRIX invView;

	// LightBuffer
	XMMATRIX lightViewProj;
	XMFLOAT3 lightDirectionVS;
	float pad0;
	XMFLOAT3 lightColor;
	float pad1;
	XMFLOAT3 ambientColor;
	float pad2;
};
static_assert(sizeof(LightingBufferType) % 16 == 0,
	"Constant buffer size must be 16-byte aligned");
