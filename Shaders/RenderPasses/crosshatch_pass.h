#pragma once

#include "render_pass.h"
#include <DirectXMath.h>

using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMVECTOR;

class CrosshatchPass : public RenderPass
{
private:
	struct CrosshatchBufferType
	{
		XMFLOAT2 offset;
		float thicknessMul;
		float topoFreqMul;

		XMFLOAT3 lightDirectionVS;
		float thresholdA;

		XMFLOAT3 inkColor;
		float thresholdB;

		XMFLOAT3 clearColor;
		float hatchAngle;

		int isFeather;
		float padding;
	};

public:
	bool SetShaderParameters(ID3D11DeviceContext*, float, float, XMFLOAT3, float*, float, float, float*, float, bool);

protected:
	const wchar_t* filename() const
	{
		return L"Shaders/crosshatch.ps";
	}

private:
	bool InitializeConstantBuffer(ID3D11Device*) override;
};