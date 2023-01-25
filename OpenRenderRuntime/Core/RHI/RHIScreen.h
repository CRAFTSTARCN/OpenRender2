#pragma once
#include <cstdint>

struct RHIRect2D
{
	uint32_t Width = 0;
	uint32_t Height = 0;
	int32_t OffsetX = 0;
	int32_t OffsetY = 0;
};

struct RHIViewport
{
	float XPos = 0.0f;
	float YPos = 0.0f;
	float Width = 0.0f;
	float Height = 0.0f;
};