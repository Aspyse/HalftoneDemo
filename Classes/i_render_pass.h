#pragma once

#include <d3d11.h>

class IRenderPass
{
public:
    virtual ~IRenderPass() = default;

    virtual bool Render(ID3D11DeviceContext* context, float* clearColor) = 0;
};