#include "SpriteEditorWorldSubsystem.h"
#include "UGUISubsystem.h"
#include "UIEditorViewport/UGUIEditorViewportInfo.h"

/////////////////////////////////////////////////////
// USpriteEditorWorldSubsystem

USpriteEditorWorldSubsystem::USpriteEditorWorldSubsystem()
{
	DesignerWidgetActor = nullptr;
}

void USpriteEditorWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &USpriteEditorWorldSubsystem::SpawnDesignerWidgetActor);
}

void USpriteEditorWorldSubsystem::Deinitialize()
{
	if (IsValid(DesignerWidgetActor) && GetWorld())
	{
		GetWorld()->DestroyActor(DesignerWidgetActor);
	}
	
	UUGUISubsystem::SetEventViewportClient(this, nullptr);
	Super::Deinitialize();
}

bool USpriteEditorWorldSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return FApp::CanEverRender() && WorldType == EWorldType::EditorPreview;
}

void USpriteEditorWorldSubsystem::SpawnDesignerWidgetActor()
{
	UWorld* World = GetWorld();
	check(World);

	bool bCanWork = false;
	for (int32 Index = 0, Count = World->ExtraReferencedObjects.Num(); Index < Count; ++Index)
	{
		const auto& Object = Cast<UUGUIEditorViewportInfo>(World->ExtraReferencedObjects[Index]);
		if (Object)
		{
			bCanWork = Object->ViewportName == TEXT("SpriteEditorViewport");
			break;
		}
	}

	if (!bCanWork)
	{
		return;
	}
	
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.bDeferConstruction = true;
		// We defer construction so that we set ParentComponent prior to component registration so they appear selected
		Params.bAllowDuringConstructionScript = true;
		Params.ObjectFlags |= RF_TextExportTransient | RF_DuplicateTransient | RF_Transient;

		const FVector Location = FVector::ZeroVector;
		const FRotator Rotation = FRotator::ZeroRotator;
		DesignerWidgetActor = Cast<ASpriteEditorWidgetActor>(
			GetWorld()->SpawnActor(ASpriteEditorWidgetActor::StaticClass(), &Location, &Rotation, Params));

		// If spawn was successful, 
		if (IsValid(DesignerWidgetActor))
		{
#if WITH_EDITOR
			DesignerWidgetActor->bRecordForUndo = false;
#endif
			// Parts that we deferred from SpawnActor
			DesignerWidgetActor->FinishSpawning(FTransform::Identity, false);
		}
	}
}

/////////////////////////////////////////////////////
