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
		XMFLOAT2 subdivisions;
		int isMonotone;
		float pad0;

		XMFLOAT3 dotColor;
		float pad1;

		XMFLOAT3 channelOffsets;
		float pad2;
	};

public:
	bool SetShaderParameters(ID3D11DeviceContext*, float*, UINT, UINT, bool, float*);

protected:
	const wchar_t* filename() const
	{
		return L"Shaders/halftone.ps";
	}

private:
	bool InitializeConstantBuffer(ID3D11Device*) override;
};