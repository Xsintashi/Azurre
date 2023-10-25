#pragma once
#include "Vector.h"

enum DLightFlags {
	DLIGHT_NO_WORLD_ILLUMINATION = 0x1,
	DLIGHT_NO_MODEL_ILLUMINATION = 0x2,
	DLIGHT_ADD_DISPLACEMENT_ALPHA = 0x4,
	DLIGHT_SUBTRACT_DISPLACEMENT_ALPHA = 0x8,
	DLIGHT_DISPLACEMENT_MASK = (DLIGHT_ADD_DISPLACEMENT_ALPHA | DLIGHT_SUBTRACT_DISPLACEMENT_ALPHA),
};

struct ColorRGBExp32
{
	byte r, g, b;
	signed char exponent;
};

struct DLight {
	int flags;
	Vector origin;
	float radius;
	ColorRGBExp32 color;
	float dieTime;
	float decay;
	float minLight;
	int	key;
	int	style;
	Vector direction;
	float innerAngle;
	float outerAngle;
};