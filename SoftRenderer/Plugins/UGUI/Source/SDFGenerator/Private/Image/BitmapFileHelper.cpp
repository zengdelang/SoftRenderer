#include "Image/BitmapFileHelper.h"
#include "Utility/PixelUtility.h"

namespace SDFGenerator
{
	template <typename T>
	static bool WriteValue(IFileHandle* FileHandle, T Value)
	{
#if PLATFORM_LITTLE_ENDIAN
		return FileHandle->Write(reinterpret_cast<const uint8*>(&Value), sizeof(T));
#else
		T Reverse = 0;
		for (int32 Index = 0; Index < sizeof(T); ++Index)
		{
			Reverse <<= 8;
			Reverse |= Value&T(0xff);
			Value >>= 8;
		}
		return FileHandle->Write((const uint8*)&Reverse, sizeof(T));
#endif
	}

	static bool WriteBmpHeader(IFileHandle* FileHandle, int32 Width, int32 Height, int32& PaddedWidth)
	{
		PaddedWidth = (3 * Width + 3) & ~3;
		constexpr uint32 BitmapStart = 54;
		const uint32 BitmapSize = PaddedWidth * Height;
		const uint32 FileSize = BitmapStart + BitmapSize;

		WriteValue<uint16>(FileHandle, 0x4d42u);
		WriteValue<uint32>(FileHandle, FileSize);
		WriteValue<uint16>(FileHandle, 0);
		WriteValue<uint16>(FileHandle, 0);
		WriteValue<uint32>(FileHandle, BitmapStart);

		WriteValue<uint32>(FileHandle, 40);
		WriteValue<int32>(FileHandle, Width);
		WriteValue<int32>(FileHandle, Height);
		WriteValue<uint16>(FileHandle, 1);
		WriteValue<uint16>(FileHandle, 24);
		WriteValue<uint32>(FileHandle, 0);
		WriteValue<uint32>(FileHandle, BitmapSize);
		WriteValue<uint32>(FileHandle, 2835);
		WriteValue<uint32>(FileHandle, 2835);
		WriteValue<uint32>(FileHandle, 0);
		WriteValue<uint32>(FileHandle, 0);
	
		return true;
	}

	bool FBitmapFileHelper::SaveBmp(const TArray<uint8>& BitmapData, int32 Width, int32 Height, const FString& Filename)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		IFileHandle* FileHandle = PlatformFile.OpenWrite(*Filename);
		if (!FileHandle)
		{
			return false;
		}
	
		int32 PaddedWidth;
		WriteBmpHeader(FileHandle, Width, Height, PaddedWidth);

		constexpr uint8 Padding = 0;
		const int32 PadLength = PaddedWidth - 3 * Width;
	
		for (int32 Y = 0; Y < Height; ++Y)
		{
			for (int32 X = 0; X < Width; ++X)
			{
				const uint8* Px = &BitmapData[Width * Y + X];
				FileHandle->Write(Px, sizeof(uint8));
				FileHandle->Write(Px, sizeof(uint8));
				FileHandle->Write(Px, sizeof(uint8));
			}

			for (int32 Index = 0; Index < PadLength; ++Index)
			{
				FileHandle->Write(&Padding, sizeof(uint8));
			}
		}

		delete FileHandle;
		return true;
	}

	bool FBitmapFileHelper::SaveBmp(const FBitmapConstRef<uint8, 1>& Bitmap, const FString& Filename)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		IFileHandle* FileHandle = PlatformFile.OpenWrite(*Filename);
		if (!FileHandle)
		{
			return false;
		}
	
		int32 PaddedWidth;
		WriteBmpHeader(FileHandle, Bitmap.Width, Bitmap.Height, PaddedWidth);

		constexpr uint8 Padding = 0;
		const int32 PadLength = PaddedWidth - 3 * Bitmap.Width;
	
		for (int32 Y = 0; Y < Bitmap.Height; ++Y)
		{
			for (int32 X = 0; X < Bitmap.Width; ++X)
			{
				const uint8* Px = Bitmap(X, Y);
				FileHandle->Write(Px, sizeof(uint8));
				FileHandle->Write(Px, sizeof(uint8));
				FileHandle->Write(Px, sizeof(uint8));
			}

			for (int32 Index = 0; Index < PadLength; ++Index)
			{
				FileHandle->Write(&Padding, sizeof(uint8));
			}
		}

		delete FileHandle;
		return true;
	}

	bool FBitmapFileHelper::SaveBmp(const FBitmapConstRef<uint8, 3>& Bitmap, const FString& Filename)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		IFileHandle* FileHandle = PlatformFile.OpenWrite(*Filename);
		if (!FileHandle)
		{
			return false;
		}
	
		int32 PaddedWidth;
		WriteBmpHeader(FileHandle, Bitmap.Width, Bitmap.Height, PaddedWidth);

		constexpr uint8 Padding = 0;
		const int32 PadLength = PaddedWidth - 3 * Bitmap.Width;
	
		for (int32 Y = 0; Y < Bitmap.Height; ++Y)
		{
			for (int32 X = 0; X < Bitmap.Width; ++X)
			{
				const uint8* Px = Bitmap(X, Y);
				FileHandle->Write(Px, 3 * sizeof(uint8));
			}

			for (int32 Index = 0; Index < PadLength; ++Index)
			{
				FileHandle->Write(&Padding, sizeof(uint8));
			}
		}

		delete FileHandle;
		return true;
	}

	bool FBitmapFileHelper::SaveBmp(const FBitmapConstRef<float>& Bitmap, const FString& Filename)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		IFileHandle* FileHandle = PlatformFile.OpenWrite(*Filename);
		if (!FileHandle)
		{
			return false;
		}
	
		int32 PaddedWidth;
		WriteBmpHeader(FileHandle, Bitmap.Width, Bitmap.Height, PaddedWidth);

		constexpr uint8 Padding = 0;
		const int32 PadLength = PaddedWidth - 3 * Bitmap.Width;
	
		for (int32 Y = 0; Y < Bitmap.Height; ++Y)
		{
			for (int32 X = 0; X < Bitmap.Width; ++X)
			{
				const uint8 Px = (const uint8)FPixelUtility::PixelFloatToByte(*Bitmap(X, Y));
				FileHandle->Write(&Px, sizeof(uint8));
				FileHandle->Write(&Px, sizeof(uint8));
				FileHandle->Write(&Px, sizeof(uint8));
			}

			for (int32 Index = 0; Index < PadLength; ++Index)
			{
				FileHandle->Write(&Padding, sizeof(uint8));
			}
		}

		delete FileHandle;
		return true;
	}

	bool FBitmapFileHelper::SaveBmp(const FBitmapConstRef<float, 3>& Bitmap, const FString& Filename)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		IFileHandle* FileHandle = PlatformFile.OpenWrite(*Filename);
		if (!FileHandle)
		{
			return false;
		}
	
		int32 PaddedWidth;
		WriteBmpHeader(FileHandle, Bitmap.Width, Bitmap.Height, PaddedWidth);

		constexpr uint8 Padding = 0;
		const int32 PadLength = PaddedWidth - 3 * Bitmap.Width;
	
		for (int32 Y = 0; Y < Bitmap.Height; ++Y)
		{
			for (int32 X = 0; X < Bitmap.Width; ++X)
			{
				const uint8 BGR[3] = {
					(uint8) FPixelUtility::PixelFloatToByte(Bitmap(X, Y)[2]),
					(uint8) FPixelUtility::PixelFloatToByte(Bitmap(X, Y)[1]),
					(uint8) FPixelUtility::PixelFloatToByte(Bitmap(X, Y)[0])
				};
				FileHandle->Write(BGR, 3 * sizeof(uint8));
			}

			for (int32 Index = 0; Index < PadLength; ++Index)
			{
				FileHandle->Write(&Padding, sizeof(uint8));
			}
		}

		delete FileHandle;
		return true;
	}
}