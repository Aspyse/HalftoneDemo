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
		int dotSize;

		XMFLOAT3 dotColor;
		float pad1;

		XMFLOAT3 channelOffsets;
		float pad2;

		HalftoneBufferType() :
			channelOffsets{ 0.26f, -0.26f, 0.0f },
			dotColor{ 1, 1, 1 },
			dotSize(6)
		{ }
	};

public:
	HalftonePass()
	{
		m_inputs.resize(1);
		m_outputs = {
			"halftone_out"
		};
	}

	std::vector<ParameterControl> GetParameters() override;
	void Update(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, XMVECTOR, XMFLOAT3, UINT, UINT) override;

protected:
	const wchar_t* filename() const
	{
		return L"Shaders/halftone.ps";
	}

private:
	bool InitializeConstantBuffer(ID3D11Device*) override;

private:
	HalftoneBufferType m_halftoneBuffer;
};