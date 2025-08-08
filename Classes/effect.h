#pragma once

#include "render_parameters.h"

#include "render_pass.h"
#include "lighting_pass.h"
#include "halftone_pass.h"
#include "blend_pass.h"
#include "crosshatch_pass.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>
#include <memory>

class Effect
{
public:
	Effect(ID3D11Device*, ID3D11DeviceContext*);

	void Initialize(std::vector<ComPtr<ID3D11ShaderResourceView>>, UINT, UINT);
	
	const std::unordered_map<std::string, std::unique_ptr<RenderTarget>>& GetTargets() const;
	const vector<std::unique_ptr<RenderPass>>& GetPasses() const;

	void AddPass(std::unique_ptr<RenderPass>);

	void Update(XMMATRIX, XMMATRIX, XMMATRIX, XMVECTOR, XMFLOAT3);
	void Render(ID3D11SamplerState*);
	void Resize(UINT, UINT);

private:
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;
	UINT m_screenWidth = 0, m_screenHeight = 0;

	vector<std::unique_ptr<RenderPass>> m_passes;
	std::unordered_map<std::string, std::unique_ptr<RenderTarget>> m_targets;
};