#pragma once
#include <DirectXMath.h>

// TODO: 
struct RenderParameters
{
    float verticalFOV;
    float nearZ;
    float farZ;
    
    float celThreshold;
    float roughness;
    float albedoColor[3];

    int halftoneDotSize;

    float edgeThreshold;
    float inkColor[3];

    float lightDirection[3];
    float clearColor[3];
    float ambientStrength;

    float thresholdA;
    float thresholdB;
    float thicknessMul;
    float densityMul;
    float hatchAngle;
    bool isFeather;

    RenderParameters() :
        verticalFOV(80),
        nearZ(1),
        farZ(1000),

        celThreshold(0.2f),
        roughness(0.16f),
        albedoColor{ 0.4f, 0.4f, 0.4f },

        halftoneDotSize(6),

        edgeThreshold(0.25),
        inkColor{ 0.0f, 0.0f, 0.0f },

        lightDirection{ 0.1f, -1.0f, 0.05f },
        clearColor{ 0.9f, 1.0f, 1.0f },
        ambientStrength(0.8f),

        thresholdA(0.35),
        thresholdB(0.60),
        thicknessMul(1),
        densityMul(1),
        hatchAngle(0.785f),
        isFeather(true)
    {}
};