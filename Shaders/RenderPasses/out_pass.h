#pragma once

#include "render_pass.h"

class OutPass : public RenderPass
{
public:
	std::vector<RenderPass::ParameterControl> GetParameters()
	{
		return {
			{ "Out", RenderPass::WidgetType::RENDER_TARGET, std::ref(m_inputs[0]) }
		};
	}

	OutPass()
	{
		m_inputs.resize(1);
	}

	void Update(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, XMVECTOR, XMFLOAT3, UINT, UINT) override {};
private:
	const wchar_t* filename() const
	{
		return L"Shaders/passthrough.ps";
	}
	bool InitializeConstantBuffer(ID3D11Device*) override { return true; };
};