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
		XMFLOAT3 lightDirectionVS;
		float thresholdA;

		XMFLOAT3 inkColor;
		float thresholdB;

		XMFLOAT3 clearColor;
		float hatchAngle;

		float thicknessMul;
		float topoFreqMul;
		int isFeather;
		float padding;

		CrosshatchBufferType() :
			thresholdA(0.35),
			thresholdB(0.6),
			thicknessMul(1),
			hatchAngle(0.785),
			inkColor{ 0.5, 0.5, 0.5 }
		{ }
	};

public:
	CrosshatchPass()
	{
		m_inputs.resize(1);
		m_outputs = {
			"crosshatch_out"
		};
	}

	void Update(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, XMVECTOR, XMFLOAT3, UINT, UINT) override;
	std::vector<ParameterControl> GetParameters() override;

protected:
	const wchar_t* filename() const
	{
		return L"Shaders/crosshatch.ps";
	}

private:
	bool InitializeConstantBuffer(ID3D11Device*) override;

	CrosshatchBufferType m_crosshatchBuffer;
};