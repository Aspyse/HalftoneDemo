#pragma once

#include "lighting_pass.h"

class FlatPass : public LightingPass
{
public:
	std::vector<RenderPass::ParameterControl> GetParameters() override
	{
		return {
			{ "Albedo", RenderPass::WidgetType::RENDER_TARGET, std::ref(m_inputs[0]) },
			{ "Normal + Roughness", RenderPass::WidgetType::RENDER_TARGET, std::ref(m_inputs[1]) },
			{ "Depth", RenderPass::WidgetType::RENDER_TARGET, std::ref(m_inputs[2]) },
			{ "Shadow Map", RenderPass::WidgetType::RENDER_TARGET, std::ref(m_inputs[3]) },
			{ "Cel Threshold", RenderPass::WidgetType::FLOAT, std::ref(m_lightBuffer.celThreshold) },
			{ "Light Color", RenderPass::WidgetType::COLOR, std::ref(m_lightBuffer.lightColor) },
		};
	}

	FlatPass()
	{
		m_inputs.resize(4);
		m_outputs = {
			"cel_out"
		};
	}
protected:
	const wchar_t* filename() const override
	{
		return L"Shaders/flat.ps";
	}
};