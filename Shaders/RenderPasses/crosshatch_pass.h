#pragma once

#include "render_pass.h"
#include <DirectXMath.h>

class CrosshatchPass : public RenderPass
{
private:
	struct CrosshatchBufferType
	{
		
	};

public:
	bool SetShaderParameters(ID3D11DeviceContext*);

private:
	bool InitializeConstantBuffer(ID3D11Device*) override { return true; };
};