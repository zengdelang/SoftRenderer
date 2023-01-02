#include "UISequenceComponent.h"
#include "UISequence.h"
#include "UISequenceModule.h"
#include "Core/WidgetActor.h"

/////////////////////////////////////////////////////
// UUISequenceComponent

UUISequenceComponent::UUISequenceComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	
#if WITH_EDITORONLY_DATA
	if (HasAnyFlags(RF_ClassDefaultObject) || GetArchetype() == GetDefault<UUISequenceComponent>())
	{
		Sequence = ObjectInitializer.CreateEditorOnlyDefaultSubobject<UUISequence>(this, "Sequence");
		if (Sequence)
		{
			Sequence->SetFlags(RF_Public | RF_Transactional);
		}
	}
#endif

	LoopType = EUISequenceLoopType::Normal;
	AutoPlayMode = EUISequenceAutoPlayMode::Once;
	
	StartTimeOffset = 0;
	Duration = 0;
	PlayRate = 1;
	
	AutoPlayLoopCount = 1;
	LoopCount = 1;

	Status = EUISequencePlayerStatus::Stopped;

	LastStopElapsedTime = 0;
	ElapsedTime = 0;
	PauseOnTime = -1;
	LastTimeSeconds = 0;
	CurPlayRate = 1;
	
	bAutoPlay = false;
	bIgnoreTimeScale = true;
	bInitialized = false;
	bReversePlayback = false;
	bPauseWhenOwnerDisabled = true;
	bNeedSetTimer = false;

#if WITH_EDITORONLY_DATA
	bConvertCubicToLinear = false;
#endif
}

void UUISequenceComponent::OnRegister()
{
	Super::OnRegister();

	CurPlayRate = PlayRate;

	if (AWidgetActor* WidgetActor = Cast<AWidgetActor>(GetOwner()))
	{
		WidgetActor->OnActorEnableStateChanged.AddUObject(this, &UUISequenceComponent::OnActorEnableStateChanged);
	}
}

void UUISequenceComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoPlay)
	{
		switch (AutoPlayMode)
		{
		case EUISequenceAutoPlayMode::Once:
			Play();
			break;
		case EUISequenceAutoPlayMode::ReverseOnce:
			PlayReverse();
			break;
		case EUISequenceAutoPlayMode::LoopExactly:
			PlayLooping(AutoPlayLoopCount);
			break;
		case EUISequenceAutoPlayMode::LoopIndefinitely:
			PlayLooping(-1);
			break;
		}
	}

	if (const AWidgetActor* WidgetActor = Cast<AWidgetActor>(GetOwner()))
	{
		OnActorEnableStateChanged(WidgetActor->bActorEnabled);
	}
}

void UUISequenceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	InternalStop();
	Super::EndPlay(EndPlayReason);
}

void UUISequenceComponent::OnUnregister()
{
	if (AWidgetActor* WidgetActor = Cast<AWidgetActor>(GetOwner()))
	{
		WidgetActor->OnActorEnableStateChanged.RemoveAll(this);
	}
	
	InternalStop();
	Evaluations.Empty();
	
	Super::OnUnregister();
}

void UUISequenceComponent::InitSetterFunctions()
{
	InitBoolSetterFunctions();
	InitFloatSetterFunctions();
	InitIntSetterFunctions();
	InitByteSetterFunctions();
	InitVector2SetterFunctions();
	InitVector3SetterFunctions();
	InitVector4SetterFunctions();
	InitColorSetterFunctions();
	InitRotatorSetterFunctions();
	InitTransformSetterFunctions();
	InitMaterialParameterSetterFunctions();
}

void UUISequenceComponent::FindSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track)
{
	switch (Track.TrackType)
	{
	case EUISequenceTrackType::Bool:
		FindBoolSetterFunction(Object, TrackObject, Track);
		break;
	case EUISequenceTrackType::Float:
		FindFloatSetterFunction(Object, TrackObject, Track);
		break;
	case EUISequenceTrackType::Int:
		FindIntSetterFunction(Object, TrackObject, Track);
		break;
	case EUISequenceTrackType::Byte:
		FindByteSetterFunction(Object, TrackObject, Track);
		break;
	case EUISequenceTrackType::Vector:
		switch (VectorCurves[Track.SectionIndex].SectionType)
		{
		case EUISequenceVectorSectionType::Vector2:
			FindVector2SetterFunction(Object, TrackObject, Track);
			break;
		case EUISequenceVectorSectionType::Vector3:
			FindVector3SetterFunction(Object, TrackObject, Track);
			break;
		case EUISequenceVectorSectionType::Vector4:
			FindVector4SetterFunction(Object, TrackObject, Track);
			break;
		case EUISequenceVectorSectionType::LinearColor:
			FindColorSetterFunction(Object, TrackObject, Track);
			break;
		case EUISequenceVectorSectionType::Rotator:
			FindRotatorSetterFunction(Object, TrackObject, Track);
			break;
		default:
			break;
		}
		break;
	case EUISequenceTrackType::Transform:
		FindTransformSetterFunction(Object, TrackObject, Track);
		break;
	case EUISequenceTrackType::MaterialParameter:
		FindMaterialParameterSetterFunction(Object, TrackObject, Track);
		break;
	case EUISequenceTrackType::AudioEvent:
		FindAudioEventSetterFunction(Object,TrackObject,Track);
		break;
	default:
		check(false);  // never run here
		break;		
	}
}

DECLARE_CYCLE_STAT(TEXT("UISequence --- Initialize"), STAT_UISequence_Initialize, STATGROUP_UISequence);
void UUISequenceComponent::Initialize()
{
	SCOPE_CYCLE_COUNTER(STAT_UISequence_Initialize);
	
	if (!bInitialized)
	{
		AActor* Owner = GetOwner();
		if (!IsValid(Owner))
			return;
		
		bInitialized = true;
		
		SequenceObjects.Empty();
		
		const UClass* OwnerClass = Owner->GetClass();
		for (const auto& Binding : Bindings)
		{
			UObject* Object = nullptr;
			if (Binding.BindingName == TEXT("Owner"))
			{
				Object = Owner;
			}
			else
			{
				const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(OwnerClass->FindPropertyByName(Binding.BindingName));
				if (ObjectProperty)
				{
					Object = ObjectProperty->GetObjectPropertyValue_InContainer(Owner);
				}
			}

			if (IsValid(Object))
			{
				FUISequenceBindObject BindObject;
				BindObject.RuntimeObj = Object;
					
				const UClass* ObjectClass = Object->GetClass();
					
				for (const auto& Track : Binding.Tracks)
				{
					static const FString Set(TEXT("Set"));

					FString TrackStr = Track.TrackName.ToString();
					if (Track.TrackType == EUISequenceTrackType::Bool && TrackStr.Len() > 0 && TrackStr[0] == 'b')
					{
						TrackStr.MidInline(1);
					}
						
					const FString FunctionString = Set + TrackStr;
					UFunction* SetterFunc = ObjectClass->FindFunctionByName(FName(*FunctionString));
						
					if (SetterFunc || Track.TrackType == EUISequenceTrackType::MaterialParameter || (Track.TrackType == EUISequenceTrackType::Transform && Track.TrackName == TEXT("Transform")))
					{
						FUISequenceTrackObject TrackObject;
						TrackObject.SetterFunction = SetterFunc;
						TrackObject.SectionIndex = Track.SectionIndex;

						FindSetterFunction(Object, TrackObject, Track);
							
						BindObject.Tracks.Emplace(TrackObject);
					}
					else if (FProperty* Property = ObjectClass->FindPropertyByName(Track.TrackName))
					{
						FUISequenceTrackObject TrackObject;
						TrackObject.Property = Property;
						TrackObject.SectionIndex = Track.SectionIndex;
						
						FindSetterFunction(Object, TrackObject, Track);
						
						BindObject.Tracks.Emplace(TrackObject);
					}
					else if(Track.TrackType == EUISequenceTrackType::AudioEvent)
					{
						FUISequenceTrackObject TrackObject;
						TrackObject.SectionIndex = Track.SectionIndex;
						
						FindSetterFunction(Object, TrackObject, Track);
						BindObject.Tracks.Emplace(TrackObject);
					}
					/*else if (Track.TrackType == EUISequenceTrackType::AkAudioEvent)
					{
						FUISequenceTrackObject TrackObject;
						TrackObject.SectionIndex = Track.SectionIndex;
						FindSetterFunction(Object, TrackObject, Track);
						BindObject.Tracks.Emplace(TrackObject);
					}*/
					else
					{
						UE_LOG(LogUISequence, Error, TEXT("UUISequenceComponent::Initialize--- Setter function or property not found, Track name: %s, Function name: %s, Property name: %s"),
							*Track.TrackName.ToString(), *FunctionString, *Track.TrackName.ToString());
					}
				}

				if (BindObject.Tracks.Num() > 0)
				{
					SequenceObjects.Emplace(BindObject);
				}
			}
			else if (Binding.bMaterTrack)
			{
				Object = Owner;
				FUISequenceBindObject BindObject;
				BindObject.RuntimeObj = Object;
				
				for (const auto& Track : Binding.Tracks)
				{
					if(Track.TrackType == EUISequenceTrackType::AudioEvent)
					{
						FUISequenceTrackObject TrackObject;
						TrackObject.SectionIndex = Track.SectionIndex;
						
						FindSetterFunction(Object, TrackObject, Track);
						BindObject.Tracks.Emplace(TrackObject);
					}
					/*if (Track.TrackType == EUISequenceTrackType::AkAudioEvent)
					{
						FUISequenceTrackObject TrackObject;
						TrackObject.SectionIndex = Track.SectionIndex;
						FindSetterFunction(Object, TrackObject, Track);
						BindObject.Tracks.Emplace(TrackObject);
					}*/
				}

				if (BindObject.Tracks.Num() > 0)
				{
					SequenceObjects.Emplace(BindObject);
				}
			}
			else
			{
				UE_LOG(LogUISequence, Error, TEXT("UUISequenceComponent::Initialize--- Binding object not found, Binding name: %s"), *Binding.BindingName.ToString());
			}
		}
	}
}

void UUISequenceComponent::Play()
{
	if (Status != EUISequencePlayerStatus::Stopped && bReversePlayback)
	{
		ElapsedTime = Duration - ElapsedTime;
	}
	
	bReversePlayback = false;
	LoopCount = 1;
	InternalPlay();
}

void UUISequenceComponent::PlayReverse()
{
	if (Status != EUISequencePlayerStatus::Stopped && !bReversePlayback)
	{
		ElapsedTime = Duration - ElapsedTime;
	}
	
	bReversePlayback = true;
	LoopCount = 1;
	InternalPlay();
}

void UUISequenceComponent::PlayLooping(int32 NumLoops)
{
	if (Status != EUISequencePlayerStatus::Stopped && bReversePlayback)
	{
		ElapsedTime = Duration - ElapsedTime;
	}
	
	bReversePlayback = false;
	LoopCount = NumLoops;
	InternalPlay();
}

void UUISequenceComponent::Pause()
{
	if (Status == EUISequencePlayerStatus::Playing)
	{
		Status = EUISequencePlayerStatus::Paused;

		ClearTimer();
		
		PauseOnTime = -1;

		if (OnPause.IsBound())
		{
			OnPause.Broadcast();
		}
	}
}

void UUISequenceComponent::Stop()
{
	if (Status != EUISequencePlayerStatus::Stopped)
	{
		InternalStop();

		if (OnStop.IsBound())
		{
			OnStop.Broadcast();
		}
	}
}

void UUISequenceComponent::GoToBeginAndStop()
{
	if (Duration <= 0 || (FMath::IsNearlyEqual(LastStopElapsedTime, 0) && Status == EUISequencePlayerStatus::Stopped))
	{
		return;
	}
	
	Initialize();

	bReversePlayback = false;
	ElapsedTime = 0;
	InternalUpdate();
	InternalStop();

	if (OnStop.IsBound())
	{
		OnStop.Broadcast();
	}
}

void UUISequenceComponent::GoToEndAndStop()
{
	if (Duration <= 0 || (FMath::IsNearlyEqual(LastStopElapsedTime, Duration) && Status == EUISequencePlayerStatus::Stopped))
	{
		return;
	}

	Initialize();

	bReversePlayback = false;
	ElapsedTime = Duration;
	InternalUpdate();
	InternalStop();
	
	if (OnStop.IsBound())
	{
		OnStop.Broadcast();
	}
}

void UUISequenceComponent::PlayTo(float InTime)
{
	PauseOnTime = FMath::Clamp(InTime, 0.0f, Duration);

	if (bReversePlayback)
	{
		const float RealElapsedTime = Duration - ElapsedTime;
		if (RealElapsedTime < PauseOnTime)
		{
			ElapsedTime = RealElapsedTime;
			Play();
		}
		else
		{
			PlayReverse();
		}
	}
	else
	{
		if (ElapsedTime < PauseOnTime)
		{
			Play();
		}
		else
		{
			ElapsedTime = Duration - ElapsedTime;
			PlayReverse();
		}
	}
}

void UUISequenceComponent::SetPlaybackPosition(float InTime)
{
	if (Duration <= 0)
		return;
	
	Initialize();
	ElapsedTime = FMath::Clamp(InTime, 0.0f, Duration);
	
	for (const auto& Evaluation : Evaluations)
	{
		Evaluation->OnEndValuation();
	}
	
	InternalUpdate();
}

void UUISequenceComponent::SetPlayRate(float InPlayRate)
{
	CurPlayRate = FMath::Max(0.0f, InPlayRate);
}

void UUISequenceComponent::InternalPlay()
{
	if (Status == EUISequencePlayerStatus::Playing || Duration <= 0)
	{
		return;
	}

	if (LoopCount == 0)
	{
		return;
	}

	const auto World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	
	Initialize();

	Status = EUISequencePlayerStatus::Playing;
	LastTimeSeconds = GetTimeSeconds(World);

	if (bReversePlayback)
	{
		if (OnPlayReverse.IsBound())
		{
			OnPlayReverse.Broadcast();
		}
	}
	else
	{
		if (OnPlay.IsBound())
		{
			OnPlay.Broadcast();
		}
	}

	bNeedSetTimer = true;
	Update();
	World->GetTimerManager().SetTimer(TimerHandle, this, &UUISequenceComponent::Update, 0.001f, false);
}

void UUISequenceComponent::InternalStop()
{
	Status = EUISequencePlayerStatus::Stopped;
	
	LoopCount = 0;
	LastStopElapsedTime = ElapsedTime;

	ElapsedTime = 0;
	PauseOnTime = -1;
	LastTimeSeconds = 0;

	ClearTimer();
}

DECLARE_CYCLE_STAT(TEXT("UISequence --- Update"), STAT_UISequence_Update, STATGROUP_UISequence);
void UUISequenceComponent::Update()
{
	SCOPE_CYCLE_COUNTER(STAT_UISequence_Update);

	const auto World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	
	bool bNeedPause = false;
	const bool bCheckPause = PauseOnTime >= 0;
	const float PreTime = bCheckPause ? GetTime(PauseOnTime) : 0;
	
	const float CurrentTimeSeconds = GetTimeSeconds(World);
	ElapsedTime += (CurrentTimeSeconds - LastTimeSeconds) * CurPlayRate;
	LastTimeSeconds = CurrentTimeSeconds;

	const float DeltaTime = ElapsedTime - Duration;
	if (DeltaTime >= 0)
	{
		if (LoopCount > 0)
		{
			--LoopCount;
		}
		
		if (LoopCount == 0)
		{
			ElapsedTime = Duration;
		}
		else
		{
			ElapsedTime = DeltaTime;

			if (LoopType == EUISequenceLoopType::PingPong)
			{
				bReversePlayback = !bReversePlayback;
			}
		}
		
		for (const auto& Evaluation : Evaluations)
		{
			Evaluation->OnEndValuation();
		}
	}

	const float CurTime = bCheckPause ? GetTime(ElapsedTime) : 0;
	if (bCheckPause)
	{
		if (bReversePlayback)
		{
			bNeedPause = CurTime <= PreTime;
		}
		else
		{
			bNeedPause = CurTime >= PreTime;
		}
	}
	
	if (bNeedPause)
	{
		ElapsedTime = PauseOnTime;
		InternalUpdate();
		Pause();
		return;
	}
	
	InternalUpdate();
		
	if (LoopCount == 0)
	{
		InternalStop();
			
		if (OnFinished.IsBound())
		{
			OnFinished.Broadcast();
		}
	}

	if (bNeedSetTimer)
	{
		World->GetTimerManager().SetTimer(TimerHandle, this, &UUISequenceComponent::Update, 0.001f, false);
	}
}

void UUISequenceComponent::InternalUpdate()
{
	for (int32 BindingIndex = 0, Count = SequenceObjects.Num(); BindingIndex < Count; ++BindingIndex)
	{
		FUISequenceBindObject& Binding = SequenceObjects[BindingIndex];
		for (int32 TrackIndex = 0, TrackCount = Binding.Tracks.Num(); TrackIndex < TrackCount; ++TrackIndex)
		{
			FUISequenceTrackObject& Track = Binding.Tracks[TrackIndex];
			(*Track.FunctionPtr)(Binding.RuntimeObj, Track.SetterFunction.Get(), Track.Property, Track.SectionIndex, this, GetTime(ElapsedTime));
		}
	}
}

void UUISequenceComponent::ClearTimer()
{
	bNeedSetTimer = false;

	for (const auto& Evaluation : Evaluations)
	{
		Evaluation->Stop();
	}
	
	if (TimerHandle.IsValid())
	{
		if (const auto World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
		}
	}
}

void UUISequenceComponent::Resume()
{
	Status = EUISequencePlayerStatus::Playing;

	const auto World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	bNeedSetTimer = true;
	LastTimeSeconds = GetTimeSeconds(World);
	Update();
	World->GetTimerManager().SetTimer(TimerHandle, this, &UUISequenceComponent::Update, 0.001f, false);
}

void UUISequenceComponent::OnActorEnableStateChanged(bool bIsEnabled)
{
	if (!bPauseWhenOwnerDisabled)
	{
		return;
	}

	if (bIsEnabled)
	{
		if (IsPaused())
		{
			Resume();
		}
	}
	else
	{
		if (IsPlaying())
		{
			Pause();
		}
	}
}

/////////////////////////////////////////////////////
