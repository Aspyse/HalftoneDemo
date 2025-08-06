#pragma once

#include "render_pass.h"
#include <DirectXMath.h>

using DirectX::XMMATRIX;
using DirectX::XMFLOAT3;

class LightingPass : public RenderPass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX invProj;
		XMMATRIX invView;

	};
	struct LightBufferType
	{
		XMMATRIX lightViewProj;
		XMFLOAT3 lightDirectionVS;
		float celThreshold;
		XMFLOAT3 lightColor;
		float pad1;
		XMFLOAT3 ambientColor;
		float pad2;

		LightBufferType() :
			lightColor(4, 4, 4)
		{ }
	};

public:
	LightingPass()
	{
		m_inputs.resize(4);
	}

	void Begin(ID3D11Device*, ID3D11DeviceContext*) override;
	void Update(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, XMVECTOR, XMFLOAT3, UINT, UINT) override;
	std::vector<ParameterControl> GetParameters() override;

protected:
	const wchar_t* filename() const override
	{
		return L"Shaders/base.ps";
	}
	const std::vector<std::string> outputs() const override
	{
		return {
			"lighting_out"
		};
	}

private:
	bool InitializeConstantBuffer(ID3D11Device*) override;

	MatrixBufferType m_matrixBuffer;
	LightBufferType m_lightBuffer;
};