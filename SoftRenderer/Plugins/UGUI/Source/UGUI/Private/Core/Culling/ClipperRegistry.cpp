#include "Core/Culling/ClipperRegistry.h"
#include "Core/Culling/ClipperInterface.h"
#include "UGUI.h"

/////////////////////////////////////////////////////
// FClipperRegistry

TSharedPtr< FClipperRegistry > FClipperRegistry::Instance = nullptr;
 
void FClipperRegistry::Initialize()
{
    if (Instance.IsValid())
    {
        return;
    }

    Instance = MakeShareable(new FClipperRegistry());
	Instance->bIsDirty = true;
}

void FClipperRegistry::Shutdown()
{
	if (Instance.IsValid())
	{
		Instance.Reset();
	}
}

DECLARE_CYCLE_STAT(TEXT("UICanvasUpdate --- Cull"), STAT_UnrealGUI_Cull, STATGROUP_UnrealGUI);
void FClipperRegistry::Cull()
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Cull);
	
	for (int32 Index = 0, Count = Clippers.Num(); Index < Count; ++Index)
	{
		const auto& ClipperObj = Clippers[Index];
		const auto Clipper = Cast<IClipperInterface>(ClipperObj);
		if (Clipper)
		{
			Clipper->PerformClipping();
		}
		else
		{
			Clippers.RemoveAt(Index);
		}
	}

	bIsDirty = false;
}

void FClipperRegistry::Register(IClipperInterface* Clipper)
{
	if (Clipper == nullptr)
		return;

	Initialize();

	if (!Instance->IsDirty())
	{
		Instance->bIsDirty = true;
	}
	
	Instance->Clippers.AddUnique(Cast<UObject>(Clipper));
}

void FClipperRegistry::Unregister(IClipperInterface* Clipper)
{
	if (Clipper == nullptr)
		return;

	Initialize();
	Instance->Clippers.Remove(Cast<UObject>(Clipper));
}

/////////////////////////////////////////////////////
