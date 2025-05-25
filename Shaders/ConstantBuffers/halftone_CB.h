#pragma once

struct HalftoneBufferType
{
	float dotSize;
	float padding[3];
};
static_assert(sizeof(LightingBufferType) % 16 == 0,
	"Constant buffer size must be 16-byte aligned");
