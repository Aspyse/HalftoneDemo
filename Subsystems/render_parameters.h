#pragma once
#include <DirectXMath.h>

// TODO: 
struct RenderParameters
{
    float verticalFOV;
    float nearZ;
    float farZ;

    float lightDirection[3];
    float clearColor[3];
    float ambientStrength;

    RenderParameters() :
        verticalFOV(80),
        nearZ(1),
        farZ(1000),

        lightDirection{ 0.1f, -1.0f, 0.05f },
        clearColor{ 0.9f, 1.0f, 1.0f },
        ambientStrength(0.45f)
    { }
};