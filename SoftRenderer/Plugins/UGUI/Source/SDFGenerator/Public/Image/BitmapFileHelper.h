#pragma once

#include "Image/Bitmap.h"

namespace SDFGenerator
{
	/**
	 * Saves the bitmap as a BMP file.
	 */
	class FBitmapFileHelper
	{
	public:
		static bool SaveBmp(const TArray<uint8>& BitmapData, int32 Width, int32 Height, const FString& Filename);
		static bool SaveBmp(const FBitmapConstRef<uint8, 1>& Bitmap, const FString& Filename);
		static bool SaveBmp(const FBitmapConstRef<uint8, 3>& Bitmap, const FString& Filename);

		static bool SaveBmp(const FBitmapConstRef<float, 1>& Bitmap, const FString& Filename);
		static bool SaveBmp(const FBitmapConstRef<float, 3>& Bitmap, const FString& Filename);
	};
}
