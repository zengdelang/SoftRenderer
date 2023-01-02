#include "FrameBuffer.h"

/////////////////////////////////////////////////////
// UFrameBuffer

UFrameBuffer::UFrameBuffer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Width = -1;
	Height = -1;

	Texture = nullptr;
}

void UFrameBuffer::Resize(int32 InWidth, int32 InHeight)
{
	if (Width != InWidth || Height != InHeight)
	{
		Width = FMath::Max(2, InWidth);
		Height = FMath::Max(2, InHeight);
		
		Pixels.SetNum(Width * Height);

		// 所有数据初始化为0，也就是纯黑色
		FMemory::Memzero(Pixels.GetData(), Pixels.Num());
	}
}

void UFrameBuffer::Clear(FLinearColor ClearColor)
{
	if (Pixels.Num() > 0)
	{
		const uint32 Color = ClearColor.ToFColor(false).DWColor();
		for (int32 Index = 0, Count = Pixels.Num(); Index < Count; ++Index)
		{
			Pixels[Index] = Color;
		}
	}
}

void UFrameBuffer::Point(int32 X, int32 Y, FLinearColor Color)
{
	if (X >= 0 && X < Width && Y >= 0 && Y < Height)
	{
		Pixels[Y * Width + X] = Color.ToFColor(false).DWColor();
	}
}

UTexture2D* UFrameBuffer::UpdateTexture2D()
{
	if (Width <= 0 || Height <= 0)
		return nullptr;
	
	if (!IsValid(Texture))
	{
		Texture = NewObject<UTexture2D>(
			GetTransientPackage(),
			NAME_None,
			RF_Transient
			);

		Texture->SRGB = false;
		Texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
		Texture->MipLoadOptions = ETextureMipLoadOptions::OnlyFirstMip;
		Texture->NeverStream = true;

		Texture->PlatformData = new FTexturePlatformData();
		Texture->PlatformData->SizeX = Width;
		Texture->PlatformData->SizeY = Height;
		Texture->PlatformData->PixelFormat = EPixelFormat::PF_B8G8R8A8;
		
		FTexture2DMipMap* Mip = new FTexture2DMipMap();
		Texture->PlatformData->Mips.Add(Mip);
	}

	if (Texture)
	{
		Texture->PlatformData->Mips[0].SizeX = Width;
		Texture->PlatformData->Mips[0].SizeY = Height;

		Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		
		uint8* RawData = static_cast<uint8*>(Texture->PlatformData->Mips[0].BulkData.Realloc(Pixels.Num() * GPixelFormats[EPixelFormat::PF_B8G8R8A8].BlockBytes));
		FMemory::Memcpy(RawData, Pixels.GetData(), Pixels.Num() * GPixelFormats[EPixelFormat::PF_B8G8R8A8].BlockBytes);
		
		Texture->PlatformData->Mips[0].BulkData.Unlock();
		
		Texture->UpdateResource();
	}
	
	return Texture;
}

void UFrameBuffer::DrawLine(int32 StartX, int32 StartY, int32 EndX, int32 EndY)
{
	
}
/////////////////////////////////////////////////////
