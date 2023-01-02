#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "Core/UICommonDefinitions.h"
#include "Subsystems/EngineSubsystem.h"
#include "UGUISubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEditorViewportInfoChanged, const UWorld*)

struct UGUI_API FWorldViewportInfo
{
protected:
	ECanvasRenderMode CanvasRenderMode;
	FVector2D ViewportSize;
	TWeakObjectPtr<UObject> EventViewportClient;
	
public:
	FWorldViewportInfo()
		: CanvasRenderMode(ECanvasRenderMode::CanvasRenderMode_ScreenSpaceFree)
		, ViewportSize(1, 1)
		, EventViewportClient(nullptr)
	{
		
	}

public:
	ECanvasRenderMode GetRenderMode() const
	{
		return CanvasRenderMode;
	}
	
	FVector2D GetViewportSize() const
	{
		return ViewportSize;
	}

	UObject* GetEventViewportClient() const
	{
		if (EventViewportClient.IsValid())
		{
			return EventViewportClient.Get();
		}
		return nullptr;
	}

public:
	void UpdateViewportSize(const UWorld* InWorld, FVector2D InViewportSize);
	void SetRenderMode(const UWorld* InWorld, ECanvasRenderMode InRenderMode);
	void SetEventViewportClient(UObject* InEventViewportClient);
	
};

UCLASS(BlueprintType, Blueprintable)
class UGUI_API UUGUISubsystem : public UEngineSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UUGUISubsystem();

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	DECLARE_MULTICAST_DELEGATE(FOnPreWillRenderCanvases)
	FOnPreWillRenderCanvases OnPreWillRenderCanvases;

	DECLARE_MULTICAST_DELEGATE(FOnAfterSubsystemTick)
	FOnAfterSubsystemTick OnAfterSubsystemTick;

protected:
	static TMap<TWeakObjectPtr<UWorld>, FWorldViewportInfo> WorldViewportInfoList;

public:
	static FOnEditorViewportInfoChanged OnEditorViewportInfoChanged;
	
public:
	// FTickableGameObject begin
	virtual void Tick(float DeltaTime) override;
	
	virtual ETickableTickType GetTickableTickType() const override
	{
		return HasAnyFlags(RF_ClassDefaultObject)
				? ETickableTickType::Never
				: ETickableTickType::Conditional;
	}
	
	virtual bool IsTickable() const override
	{
		return true;
	}
	
	virtual bool IsTickableInEditor() const override
	{
		return true;
	}
	
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UUGUISubsystem, STATGROUP_Tickables);
	}
	// FTickableGameObject end	

public:
	static FWorldViewportInfo* GetWorldViewportInfo(const UObject* WorldContextObject, bool bAddDefaultValue = false);
	static FVector2D GetViewportSize(UObject* WorldContextObject);
	static UObject* GetEventViewportClient(const UObject* WorldContextObject);
	static void SetEventViewportClient(const UObject* WorldContextObject, UObject* InEventViewportClient);
		
};
