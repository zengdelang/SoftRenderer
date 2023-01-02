#include "Designer/UGUIDesignerWorldSubsystem.h"
#include "Designer/Designer2DWidgetActor.h"
#include "Designer/DesignerWidgetActor.h"
#include "UGUISubsystem.h"

/////////////////////////////////////////////////////
// UUGUIDesignerWorldSubsystem

UUGUIDesignerWorldSubsystem::UUGUIDesignerWorldSubsystem()
{
	DesignerWidgetActor = nullptr;
	Designer2DWidgetActor = nullptr;
	BackgroundImageActor = nullptr;
}

void UUGUIDesignerWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UUGUIDesignerWorldSubsystem::SpawnDesignerWidgetActor);
}

void UUGUIDesignerWorldSubsystem::Deinitialize()
{
	if (IsValid(DesignerWidgetActor) && GetWorld())
	{
		GetWorld()->DestroyActor(DesignerWidgetActor);
	}

	if (IsValid(Designer2DWidgetActor) && GetWorld())
	{
		GetWorld()->DestroyActor(Designer2DWidgetActor);
	}

	if(IsValid(BackgroundImageActor) && GetWorld())
	{
		GetWorld()->DestroyActor(BackgroundImageActor);
	}
	
	UUGUISubsystem::SetEventViewportClient(this, nullptr);
	Super::Deinitialize();
}

bool UUGUIDesignerWorldSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return FApp::CanEverRender() && WorldType == EWorldType::EditorPreview;
}

void UUGUIDesignerWorldSubsystem::SpawnDesignerWidgetActor()
{
	UWorld* World = GetWorld();
	check(World);
	
	for (int32 Index = 0, Count = World->ExtraReferencedObjects.Num(); Index < Count; ++Index)
	{
		const auto& Object = Cast<UDesignerEditorEventViewportClient>(World->ExtraReferencedObjects[Index]);
		if (Object)
		{
			EventViewportClient = Object;
			break;
		}
	}

	if (!IsValid(EventViewportClient))
	{
		return;
	}

	UUGUISubsystem::SetEventViewportClient(this, EventViewportClient);
	
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.bDeferConstruction = true;
		// We defer construction so that we set ParentComponent prior to component registration so they appear selected
		Params.bAllowDuringConstructionScript = true;
		Params.ObjectFlags |= RF_TextExportTransient | RF_DuplicateTransient | RF_Transient;

		const FVector Location = FVector::ZeroVector;
		const FRotator Rotation = FRotator::ZeroRotator;
		DesignerWidgetActor = Cast<ADesignerWidgetActor>(
			GetWorld()->SpawnActor(ADesignerWidgetActor::StaticClass(), &Location, &Rotation, Params));

		// If spawn was successful, 
		if (IsValid(DesignerWidgetActor))
		{
#if WITH_EDITOR
			DesignerWidgetActor->bRecordForUndo = false;
#endif
			// Parts that we deferred from SpawnActor
			DesignerWidgetActor->FinishSpawning(FTransform::Identity, false);

			if (EventViewportClient && DesignerWidgetActor->RaycastRegionComponent)
			{
				DesignerWidgetActor->RaycastRegionComponent->ViewportClient	= EventViewportClient->ViewportClient;			
			}
		}
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
		Designer2DWidgetActor = Cast<ADesigner2DWidgetActor>(
			GetWorld()->SpawnActor(ADesigner2DWidgetActor::StaticClass(), &Location, &Rotation, Params));

		// If spawn was successful, 
		if (IsValid(Designer2DWidgetActor))
		{
#if WITH_EDITOR
			Designer2DWidgetActor->bRecordForUndo = false;
#endif
			
			// Parts that we deferred from SpawnActor
			Designer2DWidgetActor->FinishSpawning(FTransform::Identity, false);

			if (EventViewportClient && Designer2DWidgetActor->PrimitiveSelectComponent)
			{
				Designer2DWidgetActor->PrimitiveSelectComponent->ViewportClient	= EventViewportClient->ViewportClient;			
			}
		}
	}

	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.bDeferConstruction = true;
		Params.bAllowDuringConstructionScript = true;
		Params.ObjectFlags |= RF_TextExportTransient | RF_DuplicateTransient | RF_Transient;

		const FVector Location = FVector::ZeroVector;
		const FRotator Rotation = FRotator::ZeroRotator;

		BackgroundImageActor = Cast<ABackgroundImageActor>(
			GetWorld()->SpawnActor(ABackgroundImageActor::StaticClass(), &Location, &Rotation, Params));

		if(IsValid(BackgroundImageActor))
		{
			BackgroundImageActor->bRecordForUndo = false;
			BackgroundImageActor->BackgroundImageComponent->ViewportClient = EventViewportClient->ViewportClient;
			BackgroundImageActor->FinishSpawning(FTransform::Identity, false);
		}
	}
}
