#pragma once

#include "render_pass.h"
#include <DirectXMath.h>

using DirectX::XMMATRIX;
using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMVECTOR;

class BlendPass : public RenderPass
{
public:
	std::vector<RenderPass::ParameterControl> GetParameters()
	{
		return {
			{ "Foreground", RenderPass::WidgetType::RENDER_TARGET, std::ref(m_inputs[1]) },
			{ "Background", RenderPass::WidgetType::RENDER_TARGET, std::ref(m_inputs[0]) }
		};
	}

	BlendPass()
	{
		m_inputs.resize(2);
		m_outputs = { "blend_out" };
	}

	void Update(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, XMVECTOR, XMFLOAT3, UINT, UINT) override {};
private:
	const wchar_t* filename() const
	{
		return L"Shaders/blend.ps";
	}
	bool InitializeConstantBuffer(ID3D11Device*) override { return true; };
};