#pragma once
#include <DirectXMath.h>
struct RenderParameters
{
    float lightDirection[3];
    float clearColor[3];
    float ambientStrength;
    float celThreshold;
    float roughness;
    float albedoColor[3];
    int halftoneDotSize;

    RenderParameters():
        lightDirection{ 1.0f, -1.0f, -1.0f },
        clearColor{ 0.9f, 1.0f, 1.0f },
        ambientStrength(0.8f),
        celThreshold(0.4f),
        roughness(0.16f),
        albedoColor{ 1.0f, 0.25f, 0.0f },
        halftoneDotSize(6)
    {}
};