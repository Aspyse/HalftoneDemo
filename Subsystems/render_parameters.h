#pragma once
#include <DirectXMath.h>

// TODO: 
struct RenderParameters
{
    float celThreshold;
    float roughness;
    float albedoColor[3];
    int halftoneDotSize;
    float edgeThreshold;
    float inkColor[3];

    float lightDirection[3];
    float clearColor[3];
    float ambientStrength;

    RenderParameters() :
        celThreshold(0.5f),
        roughness(0.16f),
        albedoColor{ 1.0f, 0.25f, 0.0f },
        halftoneDotSize(6),
        edgeThreshold(0.25),
        inkColor{ 0.0f, 0.0f, 0.0f },
        lightDirection{ 1.0f, -1.0f, 1.0f },
        clearColor{ 0.9f, 1.0f, 1.0f },
        ambientStrength(0.8f)
    {}
};