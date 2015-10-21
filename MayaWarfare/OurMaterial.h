#pragma once
#include "gameplay.h"

using namespace gameplay;

class OurMaterial
{
public:
	Vector4 color;
	Vector4 getColor() const
	{
		return color;
	}
};