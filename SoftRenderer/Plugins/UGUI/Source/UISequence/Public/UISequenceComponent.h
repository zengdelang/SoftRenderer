#pragma once

#include "Components/ActorComponent.h"
#include "MovieSceneSequencePlayer.h"
#include "UISequenceCommonDefinitions.h"
#if WITH_EDITORONLY_DATA
#include "UISequence.h"
#endif
#include "UISequenceComponent.generated.h"

class UUISequenceComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUISequenceEvent);

typedef TFunction<void(UObject*, UFunction*, FProperty* Prop, int32, UUISequenceComponent*, float)> UISequenceInterpFunc;

enum class EUISequencePlayerStatus : uint8
{
	Stopped,
	Playing,
	Paused,
};

struct FUISequenceTrackObject
{
	TWeakObjectPtr<UFunction> SetterFunction;

	FProperty* Property;
		
	int32 SectionIndex;

	UISequenceInterpFunc* FunctionPtr;

public:
	FUISequenceTrackObject() : SectionIndex(-1)
	{
		Property = nullptr;
		FunctionPtr = nullptr;
	}
};

USTRUCT(BlueprintType)
struct FUISequenceBindObject
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	UObject* RuntimeObj;

	TArray<FUISequenceTrackObject> Tracks;

public:
	FUISequenceBindObject() : RuntimeObj(nullptr)
	{
	}
};

UENUM()
enum class EUISequenceLoopType : uint8
{
	Normal,
	PingPong,
};

UENUM()
enum class EUISequenceAutoPlayMode : uint8
{
	Once,
	ReverseOnce,
	LoopIndefinitely,
	LoopExactly,
};

/**
 * Movie scene animation embedded within an actor.
 */
UCLASS(Blueprintable, ClassGroup=Sequence, hidecategories=(Collision, Cooking, Activation), meta=(UIBlueprintSpawnableComponent, BlueprintSpawnableComponent), HideCategories=(Tags, Cooking, Physics, Collision, LOD, Rendering, Activation, ComponentReplication, AssetUserData, ComponentTick))
class UISEQUENCE_API UUISequenceComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class FUISequenceCompiler;
	friend class FUISequenceModule;
	friend class FUISequenceComponentCustomization;
	
public:
#if WITH_EDITORONLY_DATA
	/** Embedded actor sequence data */
	UPROPERTY(EditAnywhere, Instanced, Category=Animation)
	UUISequence* Sequence;
#endif

public:
	UPROPERTY(EditAnywhere, Category = "UISequence")
	EUISequenceLoopType LoopType;

	UPROPERTY(EditAnywhere, Category = "UISequence")
	EUISequenceAutoPlayMode AutoPlayMode;
	
	UPROPERTY(EditAnywhere, Category = "UISequence", meta = (ClampMin = "1", UIMin = "1"))
	int32 AutoPlayLoopCount;

	UPROPERTY(EditAnywhere, Category = "UISequence", meta = (ClampMin = "0.001", UIMin = "0.001"))
	float PlayRate;
	
	UPROPERTY(VisibleAnywhere, Category = "UISequence", AdvancedDisplay)
	float StartTimeOffset;
	
	UPROPERTY(VisibleAnywhere, Category = "UISequence", AdvancedDisplay)
	float Duration;

	UPROPERTY(VisibleAnywhere, Category = "UISequence", AdvancedDisplay)
	FFrameRate DisplayRate;

	UPROPERTY(VisibleAnywhere, Category = "UISequence", AdvancedDisplay)
	FFrameRate TickResolution;
	
	UPROPERTY(VisibleAnywhere, Category = "UISequence", AdvancedDisplay)
	FFrameRate InputRate;

	UPROPERTY(VisibleAnywhere, Category = "UISequence", AdvancedDisplay)
	TArray<FUISequenceBinding> Bindings;

	UPROPERTY(VisibleAnywhere, Category = "UISequence", AdvancedDisplay)
	TArray<FUISequenceBoolCurve> BoolCurves;

	UPROPERTY(VisibleAnywhere, Category = "UISequence", AdvancedDisplay)
	TArray<FUISequenceFloatCurve> FloatCurves;

	UPROPERTY(VisibleAnywhere, Category = "UISequence", AdvancedDisplay)
	TArray<FUISequenceIntCurve> IntCurves;
	
	UPROPERTY(VisibleAnywhere, Category = "UISequence", AdvancedDisplay)
	TArray<FUISequenceVectorCurve> VectorCurves;

	UPROPERTY(VisibleAnywhere, Category = "UISequence", AdvancedDisplay)
	TArray<FUISequenceTransformCurve> TransformCurves;

	UPROPERTY(VisibleAnywhere, Category = "UISequence", AdvancedDisplay)
	TArray<FUIMaterialParameterCurve> ParameterCurves;

	UPROPERTY(VisibleAnywhere, Category = "UISequence", AdvancedDisplay)
	TArray<FUISequenceAudioCurve> AudioCurves;
	
	UPROPERTY(Transient)
	TArray<FUISequenceBindObject> SequenceObjects;

	EUISequencePlayerStatus Status;

	FTimerHandle TimerHandle;

	int32 LoopCount;
	
	float LastStopElapsedTime;
	float ElapsedTime;
	float PauseOnTime;
	float LastTimeSeconds;
	float CurPlayRate;
	
	UPROPERTY(EditAnywhere, Category = "UISequence")
	uint8 bAutoPlay : 1;

	UPROPERTY(EditAnywhere, Category = "UISequence")
	uint8 bIgnoreTimeScale : 1;

	UPROPERTY(EditAnywhere, Category = "UISequence")
	uint8 bPauseWhenOwnerDisabled : 1;
	
	uint8 bInitialized : 1;

	uint8 bReversePlayback : 1;

	uint8 bNeedSetTimer : 1;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Editor Export")
	uint8 bConvertCubicToLinear : 1;
#endif

	TSet<TSharedPtr<IUISequenceEvaluationData>> Evaluations;

public:
	/** Event triggered when the level sequence player is played */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnUISequenceEvent OnPlay;

	/** Event triggered when the level sequence player is played in reverse */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnUISequenceEvent OnPlayReverse;

	/** Event triggered when the level sequence player is stopped */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnUISequenceEvent OnStop;

	/** Event triggered when the level sequence player is paused */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnUISequenceEvent OnPause;

	/** Event triggered when the level sequence player finishes naturally (without explicitly calling stop) */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnUISequenceEvent OnFinished;

public:
	UUISequenceComponent(const FObjectInitializer& Init);

#if WITH_EDITOR
	void ClearData()
	{
		StartTimeOffset = 0;
		Duration = 0;
		Bindings.Empty();
		BoolCurves.Empty();
		FloatCurves.Empty();
		IntCurves.Empty();
		VectorCurves.Empty();
		TransformCurves.Empty();
		ParameterCurves.Empty();
		AudioCurves.Empty();
	}
#endif

public:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnUnregister() override;
	
	bool EvaluateFloat(const FUISequenceFloatCurve& FloatCurve, float InTime, float& OutValue);

protected:
	static void InitSetterFunctions();
	void FindSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track);

	static void InitBoolSetterFunctions();
	static void FindBoolSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track);
	
	static void InitFloatSetterFunctions();
	static void FindFloatSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track);

	static void InitIntSetterFunctions();
	static void FindIntSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track);

	static void InitByteSetterFunctions();
	static void FindByteSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track);
	
	static void InitVector2SetterFunctions();
	static void FindVector2SetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track);

	static void InitVector3SetterFunctions();
	static void FindVector3SetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track);

	static void InitVector4SetterFunctions();
	static void FindVector4SetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track);

	static void InitColorSetterFunctions();
	static void FindColorSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track);

	static void InitRotatorSetterFunctions();
	static void FindRotatorSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track);

	static void InitTransformSetterFunctions();
	static void FindTransformSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track);

	static void InitMaterialParameterSetterFunctions();
	static void FindMaterialParameterSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track);
	
	void FindAudioEventSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track);
	
	void Initialize();

public:
	UFUNCTION(BlueprintCallable, Category = "UISequence")
	void Play();

	UFUNCTION(BlueprintCallable, Category = "UISequence")
	void PlayReverse();

	UFUNCTION(BlueprintCallable, Category = "UISequence")
	void PlayLooping(int32 NumLoops = -1);

	UFUNCTION(BlueprintCallable, Category = "UISequence")
	void Pause();

	UFUNCTION(BlueprintCallable, Category = "UISequence")
	void Stop();

	UFUNCTION(BlueprintCallable, Category = "UISequence")
	void GoToBeginAndStop();

	UFUNCTION(BlueprintCallable, Category = "UISequence")
	void GoToEndAndStop();

	/**
	 * Play from the current position to the requested position and pause. If requested position is before the current position, 
	 * playback will be reversed. Playback to the requested position will be cancelled if Stop() or Pause() is invoked during this 
	 * playback.
	 */
	UFUNCTION(BlueprintCallable, Category = "UISequence")
	void PlayTo(float InTime);

	/**
	 * Set the current time of the player by evaluating from the current time to the specified time, as if the sequence is playing. 
	 * Triggers events that lie within the evaluated range. Does not alter the persistent playback status of the player (IsPlaying).
	 */
	UFUNCTION(BlueprintCallable, Category = "UISequence")
	void SetPlaybackPosition(float InTime);

	UFUNCTION(BlueprintCallable, Category="Game|Cinematic")
	void SetPlayRate(float InPlayRate);
	
public:
	UFUNCTION(BlueprintCallable, Category = "UISequence")
	bool IsPlaying() const
	{
		return Status == EUISequencePlayerStatus::Playing;
	}

	UFUNCTION(BlueprintCallable, Category = "UISequence")
	bool IsPaused() const
	{
		return Status == EUISequencePlayerStatus::Paused;
	}

	UFUNCTION(BlueprintCallable, Category = "UISequence")
	bool IsReversed() const
	{
		return bReversePlayback;
	}
 
	UFUNCTION(BlueprintCallable, Category = "UISequence")
	float GetDuration() const
	{
		return Duration;
	}

	UFUNCTION(BlueprintCallable, Category = "UISequence")
	float GetElapsedTime() const
	{
		return ElapsedTime;
	}

	UFUNCTION(BlueprintCallable, Category = "UISequence")
	void SetLoopType(EUISequenceLoopType InLoopType)
	{
		LoopType = InLoopType;
	}

	float GetTime(float InTime) const
	{
		return bReversePlayback ? Duration - InTime + StartTimeOffset : StartTimeOffset + InTime;
	}
	
	const FFrameRate& GetDisplayRate() const
	{
		return DisplayRate;
	}

	const FFrameRate& GetTickResolution() const
	{
		return TickResolution;
	}

	const FFrameRate& GetInputRate() const
	{
		return InputRate;
	}

	const FUISequenceBoolCurve& GetBoolCurveData(const int32& InSectionIndex)
	{
		return BoolCurves[InSectionIndex];
	}

	const FUISequenceFloatCurve& GetFloatCurveData(const int32& InSectionIndex)
	{
		return FloatCurves[InSectionIndex];
	}

	const FUISequenceVectorCurve& GetVectorCurveData(const int32& InSectionIndex)
	{
		return VectorCurves[InSectionIndex];
	}

	const FUISequenceIntCurve& GetIntCurveData(const int32& InSectionIndex)
	{
		return IntCurves[InSectionIndex];
	}

	const FUISequenceTransformCurve& GetTransformCurve(const int32& InSectionIndex)
	{
		return TransformCurves[InSectionIndex];
	}

	const FUIMaterialParameterCurve& GetParameterCurve(const int32& InSectionIndex)
	{
		return ParameterCurves[InSectionIndex];
	}

	const FUISequenceAudioCurve& GetAudioCurve(const int32& InSectionIndex)
	{
		return AudioCurves[InSectionIndex];
	}

protected:
	FORCEINLINE float GetTimeSeconds(const UWorld* InWorld) const
	{
		return bIgnoreTimeScale ? InWorld->GetUnpausedTimeSeconds() : InWorld->GetTimeSeconds();
	}

private:
	void InternalPlay();
	void InternalStop();
	
	void Update();
	void InternalUpdate();

	void ClearTimer();

	void Resume();
	void OnActorEnableStateChanged(bool bIsEnabled);
};

template<typename T>
void InvokeSetterFunction(UObject* InRuntimeObject, UFunction* Setter, T&& InPropertyValue)
{
	// CacheBinding already guarantees that the function has >= 1 parameters
	const int32 ParamsSize = Setter->ParmsSize;

	// This should all be const really, but ProcessEvent only takes a non-const void*
	void* InputParameter = const_cast<typename TDecay<T>::Type*>(&InPropertyValue);

	// By default we try and use the existing stack value
	uint8* Params = reinterpret_cast<uint8*>(InputParameter);

	check(InRuntimeObject && Setter);
	if (Setter->ReturnValueOffset != MAX_uint16 || Setter->NumParms > 1)
	{
		// Function has a return value or multiple parameters, we need to initialize memory for the entire parameter pack
		// We use allocate here (as in UObject::ProcessEvent) to avoid a heap allocation. Allocate memory survives the current function's stack frame.
		Params = reinterpret_cast<uint8*>(FMemory_Alloca(ParamsSize));

		bool bFirstProperty = true;
		for (FProperty* Property = Setter->PropertyLink; Property; Property = Property->PropertyLinkNext)
		{
			// Initialize the parameter pack with any param properties that reside in the container
			if (Property->IsInContainer(ParamsSize))
			{
				Property->InitializeValue_InContainer(Params);

				// The first encountered property is assumed to be the input value so initialize this with the user-specified value from InPropertyValue
				if (Property->HasAnyPropertyFlags(CPF_Parm) && !Property->HasAnyPropertyFlags(CPF_ReturnParm) && bFirstProperty)
				{
					const bool bIsValid = ensureMsgf(sizeof(T) == Property->ElementSize, TEXT("Property type does not match for Sequencer setter function %s::%s (%ibytes != %ibytes"), *InRuntimeObject->GetName(), *Setter->GetName(), sizeof(T), Property->ElementSize);
					if (bIsValid)
					{
						Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>(Params), &InPropertyValue);
					}
					else
					{
						return;
					}
				}
				bFirstProperty = false;
			}
		}
	}

	// Now we have the parameters set up correctly, call the function
	InRuntimeObject->ProcessEvent(Setter, Params);
}
