#pragma once

#include "Arithmetics.h"

namespace SDFGenerator
{
	class FPixelUtility
	{
	public:
		static uint8 PixelFloatToByte(float X)
		{
			return static_cast<uint8>(Clamp(256.f * X, 255.f));
		}
		
		static float PixelByteToFloat(uint8 X)
		{
			return 1.f/255.f * X;
		}
	};
}
