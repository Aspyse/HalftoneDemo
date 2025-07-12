#pragma once

#include "render_parameters.h"

#include "render_pass.h"
#include "lighting_pass.h"
#include "halftone_pass.h"
#include "sobel_pass.h"
#include "blend_pass.h"
#include "crosshatch_pass.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>

class RenderGraph
{
public:
	RenderGraph(ID3D11Device*, ID3D11DeviceContext*, UINT, UINT);

	void DebugInit(std::shared_ptr<RenderTarget>, ID3D11RenderTargetView*, ID3D11DepthStencilView*);
	void SetParameters(RenderParameters&, XMVECTOR, XMMATRIX, XMMATRIX, XMMATRIX); // consider refactoring parameter system entirely
	void Render(ID3D11SamplerState*, RenderParameters&);
	void Resize(UINT, UINT);

private:
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;
	UINT m_screenWidth = 0, m_screenHeight = 0;

	vector<std::unique_ptr<RenderPass>> m_passes;
};