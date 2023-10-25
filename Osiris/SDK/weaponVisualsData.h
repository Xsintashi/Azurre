#pragma once

#include "Vector.h"
#include "Pad.h"

class weaponVisualsData {
public:
	char pad[0x77C];
	char weaponPath[260];
	char pad2[0x10C];

	float color1B;
	float color1R;
	float color1G;

	float color2B;
	float color2R;
	float color2G;

	float color3B;
	float color3R;
	float color3G;

	float color4B;
	float color4R;
	float color4G;

	int phongAlbedoBoost;
	int phongExponent;
	int phongIntensity;

	float phongAlbedoFactor;

	float wearProgress;

	float patternScale;
	Vector patternOffset;
	float patternRot;

	float wearFcale;
	Vector wearOffset;
	float wearRot;

	float grungeScale;
	Vector grungeOffset;
	float grungeRot;
};
