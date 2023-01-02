#include "AudioDevice.h"
#include "UISequenceComponent.h"
#include "UISequenceModule.h"
#include "Sound/SoundCue.h"
#include "ActiveSound.h"
#include "UGUIWorldSubsystem.h"

static float MaxUISequenceAudioDesyncToleranceCVar = 0.5f;
FAutoConsoleVariableRef CVarMaxUISequenceAudioDesyncTolerance(
	TEXT("UISequence.Audio.MaxDesyncTolerance"),
	MaxUISequenceAudioDesyncToleranceCVar,
	TEXT("Controls how many seconds an audio track can be out of sync in a Sequence before we attempt a time correction.\n"),
	ECVF_Default);

static bool bIgnoreUIAudioSyncDuringWorldTimeDilationCVar = true;
FAutoConsoleVariableRef CVarIgnoreUIAudioSyncDuringWorldTimeDilation(
	TEXT("UISequence.Audio.IgnoreAudioSyncDuringWorldTimeDilation"),
	bIgnoreUIAudioSyncDuringWorldTimeDilationCVar,
	TEXT("Ignore correcting audio if there is world time dilation.\n"),
	ECVF_Default);

static int32 UseAudioClockForUISequencerDesyncCVar = 0;
FAutoConsoleVariableRef CVarUseAudioClockForUISequencerDesync(
	TEXT("UISequence.Audio.UseAudioClockForAudioDesync"),
	UseAudioClockForUISequencerDesyncCVar,
	TEXT("When set to 1, we will use the audio render thread directly to query whether audio has went out of sync with the sequence.\n"),
	ECVF_Default);

struct FUISequenceAudioEvaluateData : IUISequenceEvaluationData
{
public:
	FUISequenceAudioEvaluateData(uint16 InCurveIndex, uint16 InSectionIndex) : CurveIndex(InCurveIndex), SectionIndex(InSectionIndex),
	                                                                           SoundLastPlayedAtTime(0),
	                                                                           bHasValidSoundLastPlayedAtTime(false)
	{
	}

	static UAudioComponent* GetAudioComponent(const UUISequenceComponent* SequenceComp, UObject* PrincipalObject, bool bMasterTrack)
	{
		if (const auto World = SequenceComp->GetWorld())
		{
			if (const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>())
			{
				return UIWorldSubsystem->GetAudioComponent(PrincipalObject, bMasterTrack);
			}
		}
		
		return nullptr;
	}

	static void CacheAudioComponent(UAudioComponent* AudioComponent)
	{
		if (IsValid(AudioComponent))
		{
			if (const auto World = AudioComponent->GetWorld())
			{
				if (const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>())
				{
					return UIWorldSubsystem->CacheAudioComponent(AudioComponent);
				}
			}
		}
	}

	virtual void Update(const float PlayingTime, UObject* ObjectBinding, UUISequenceComponent* SequenceComp) override
	{
		const FUISequenceAudioCurve& Curve = SequenceComp->GetAudioCurve(CurveIndex);
		const auto& AudioSection = Curve.AudioSections[SectionIndex];
		
		USoundBase* Sound = AudioSection.Sound;
		if (!Sound)
		{
			return;
		}
		
		float AudioTime = PlayingTime + AudioSection.StartFrameOffset;
		if (AudioTime >= 0.0f)
		{
			const float Duration = MovieSceneHelpers::GetSoundDuration(Sound);
			
			if (!AudioSection.bLooping && PlayingTime > Duration && Duration != 0.0f)
			{
				Stop();
				return;
			}

			AudioTime = Duration > 0.0f ? FMath::Fmod(AudioTime, Duration) : 0.0f;
		}

		if (!AudioComponent.IsValid())
		{
			AudioComponent = GetAudioComponent(SequenceComp, ObjectBinding, Curve.bMasterTrack);
			if (!AudioComponent.IsValid())
			{
				return;
			}
		}
		
		float AudioVolume = 1.f;
		SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(AudioSection.VolumeCurveIndex), PlayingTime,AudioVolume);
		if (AudioComponent->VolumeMultiplier != AudioVolume)
		{
			AudioComponent->SetVolumeMultiplier(AudioVolume);
		}
		
		float PitchMultiplier = 1.f;
		SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(AudioSection.PitchMultiplierCurveIndex), PlayingTime,PitchMultiplier);
		if (AudioComponent->PitchMultiplier != PitchMultiplier)
		{
			AudioComponent->SetPitchMultiplier(PitchMultiplier);
		}
		
		// If the audio component is not playing we (may) need a state change. If the audio component is playing
		// the wrong sound then we need a state change. If the audio playback time is significantly out of sync 
		// with the desired time then we need a state change.
		const bool bSoundNeedsStateChange = !AudioComponent->IsPlaying() || AudioComponent->Sound != Sound;
		bool bSoundNeedsTimeSync = false;

		const bool bDoTimeSync = 
			ObjectBinding->GetWorld() && 
			(FMath::IsNearlyEqual(ObjectBinding->GetWorld()->GetWorldSettings()->GetEffectiveTimeDilation(), 1.f) ||
			 !bIgnoreUIAudioSyncDuringWorldTimeDilationCVar);

		if (bDoTimeSync)
		{
			float CurrentGameTime;

			const FAudioDevice* AudioDevice = ObjectBinding->GetWorld()->GetAudioDeviceRaw();
			if (UseAudioClockForUISequencerDesyncCVar && AudioDevice)
			{
				CurrentGameTime = AudioDevice->GetAudioClock();
			}
			else
			{
				CurrentGameTime =ObjectBinding->GetWorld()->GetAudioTimeSeconds();
			}
			
			// This tells us how much time has passed in the game world (and thus, reasonably, the audio playback)
			// so if we calculate that we should be playing say, 15s into the section during evaluation, but
			// we're only 5s since the last Play call, then we know we're out of sync.
			if (bHasValidSoundLastPlayedAtTime)
			{
				const float GameTimeDelta = CurrentGameTime - SoundLastPlayedAtTime;
				if (!FMath::IsNearlyZero(MaxUISequenceAudioDesyncToleranceCVar) && FMath::Abs(GameTimeDelta - AudioTime) > MaxUISequenceAudioDesyncToleranceCVar)
				{
					bSoundNeedsTimeSync = true;
				}
			}
		}

		if (bSoundNeedsStateChange || bSoundNeedsTimeSync)
		{
			AudioComponent->bAllowSpatialization = !Curve.bMasterTrack;
			
			if (AudioSection.AttenuationSettings)
			{
				AudioComponent->AttenuationSettings = AudioSection.AttenuationSettings;
			}

			// Only call stop on the sound if it is actually playing. This prevents spamming
			// stop calls when a sound cue with a duration of zero is played.
			if (AudioComponent->IsPlaying() || bSoundNeedsTimeSync)
			{
				UE_LOG(LogUISequence, Warning, TEXT("Audio Component stopped due to needing a state change bIsPlaying: %d bNeedsTimeSync: %d. Component: %s sound: %s"), AudioComponent->IsPlaying(), bSoundNeedsTimeSync, *AudioComponent->GetName(), *AudioComponent->Sound->GetName());
				AudioComponent->Stop();
			}

			// Only change the sound clip if it has actually changed. This calls Stop internally if needed.
			if (AudioComponent->Sound != Sound)
			{
				UE_LOG(LogUISequence, Warning, TEXT("Audio Component calling SetSound due to new sound. Component: %s OldSound: %s NewSound: %s"), *AudioComponent->GetName(), *GetNameSafe(AudioComponent->Sound), *Sound->GetName());
				AudioComponent->SetSound(Sound);
			}

#if WITH_EDITOR
			const UWorld* World = ObjectBinding->GetWorld();
			if (GIsEditor && World != nullptr && !World->IsPlayInEditor())
			{
				AudioComponent->bIsUISound = true;
				AudioComponent->bIsPreviewSound = true;
			}
			else
#endif // WITH_EDITOR
			{
				AudioComponent->bIsUISound = false;
			}

			if (AudioTime >= 0.0f)
			{
				AudioComponent->Play(AudioTime);

				if (ObjectBinding->GetWorld())
				{
					bHasValidSoundLastPlayedAtTime = true;

					const FAudioDevice* AudioDevice = ObjectBinding->GetWorld()->GetAudioDeviceRaw();
					if (UseAudioClockForUISequencerDesyncCVar && AudioDevice)
					{
						SoundLastPlayedAtTime = AudioDevice->GetAudioClock() - AudioTime;
					}
					else
					{
						SoundLastPlayedAtTime = ObjectBinding->GetWorld()->GetAudioTimeSeconds() - AudioTime;
					}
				}
			}
		}

		if (!Curve.bMasterTrack)
		{
			if (FAudioDevice* AudioDevice = AudioComponent->GetAudioDevice())
			{
				DECLARE_CYCLE_STAT(TEXT("FAudioThreadTask.UISequenceUpdateAudioTransform"), STAT_UISequenceUpdateAudioTransform, STATGROUP_TaskGraphTasks);

				const FTransform ActorTransform = AudioComponent->GetComponentTransform();
				const uint64 ActorComponentID = AudioComponent->GetAudioComponentID();
				FAudioThread::RunCommandOnAudioThread([AudioDevice, ActorComponentID, ActorTransform]()
				{
					if (FActiveSound* ActiveSound = AudioDevice->FindActiveSound(ActorComponentID))
					{
						ActiveSound->bLocationDefined = true;
						ActiveSound->Transform = ActorTransform;
					}
				}, GET_STATID(STAT_UISequenceUpdateAudioTransform));
			}
		}
	}
	
	virtual void Stop() override
	{
		if (AudioComponent.IsValid())
		{
			CacheAudioComponent(AudioComponent.Get());
			AudioComponent.Reset();
		}
		
		bHasValidSoundLastPlayedAtTime = false;
		SoundLastPlayedAtTime = 0;
	}
	
	virtual void OnEndValuation() override
	{
		
	}
 
private:
	uint16 CurveIndex;
	uint16 SectionIndex;

	float SoundLastPlayedAtTime;
	bool bHasValidSoundLastPlayedAtTime;
	
	TWeakObjectPtr<UAudioComponent> AudioComponent;
};

void UpdateAudio(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	const FUISequenceAudioCurve& AudioCurve = SequenceComp->GetAudioCurve(DataIndex);
	for (const auto& AudioSection : AudioCurve.AudioSections)
	{
		if (AudioSection.EvaluationDataPtr.IsValid())
		{
			if (Time >= AudioSection.ClipStartTime && Time < AudioSection.ClipEndTime)
			{
				AudioSection.EvaluationDataPtr.Pin()->Update(Time - AudioSection.ClipStartTime, Obj, SequenceComp);
			}
			else if (Time >= AudioSection.ClipEndTime)
			{
				AudioSection.EvaluationDataPtr.Pin()->Stop();
			}
		}
	}
}

static UISequenceInterpFunc UpdateAudioFunction = UpdateAudio;

void UUISequenceComponent::FindAudioEventSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track)
{
	FUISequenceAudioCurve& AudioEventCurve = AudioCurves[Track.SectionIndex];
	for (int32 Index = 0, Count = AudioEventCurve.AudioSections.Num(); Index < Count; ++Index)
	{
		TSharedPtr<IUISequenceEvaluationData> EvaluationData = MakeShareable(new FUISequenceAudioEvaluateData(Track.SectionIndex, Index));
		AudioEventCurve.AudioSections[Index].EvaluationDataPtr = EvaluationData;
		Evaluations.Emplace(EvaluationData);
	}
	
	TrackObject.FunctionPtr = &UpdateAudioFunction;
}
