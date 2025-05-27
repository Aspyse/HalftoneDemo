#pragma once

#include "render_pass.h"

/* EFFECT DOES NOT WORK AS INTENDED :( */
class NormalEdgePass : public RenderPass
{
private:
	bool InitializeConstantBuffer(ID3D11Device*) override { return true; };
};