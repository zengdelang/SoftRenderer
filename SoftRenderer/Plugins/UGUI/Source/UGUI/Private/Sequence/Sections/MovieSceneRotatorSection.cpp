#include "Sequence/Sections/MovieSceneRotatorSection.h"
#include "UObject/StructOnScope.h"
#include "UObject/SequencerObjectVersion.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "Compilation/MovieSceneTemplateInterrogation.h"
#include "Evaluation/MovieSceneEvaluationTrack.h"
#include "Evaluation/MovieSceneEvaluationTemplateInstance.h"
#include "Evaluation/MovieScenePropertyTemplate.h"
#include "Sequence/Evaluation/MovieSceneRotatorTemplate.h"
#include "Sequence/Tracks/MovieSceneRotatorTrack.h"

#if WITH_EDITOR
struct FRotatorSectionEditorData
{
	FRotatorSectionEditorData()
	{
		MetaData[0].SetIdentifiers("Rotator.Pitch", NSLOCTEXT("MovieSceneChannels", "ChannelPitch", "Pitch"));
		MetaData[0].SortOrder = 1;
		MetaData[0].Color = FCommonChannelData::RedChannelColor;
		MetaData[0].bCanCollapseToTrack = false;

		MetaData[1].SetIdentifiers("Rotator.Yaw", NSLOCTEXT("MovieSceneChannels", "ChannelYaw", "Yaw"));
		MetaData[1].SortOrder = 2;
		MetaData[1].Color = FCommonChannelData::GreenChannelColor;
		MetaData[1].bCanCollapseToTrack = false;

		MetaData[2].SetIdentifiers("Rotator.Roll", NSLOCTEXT("MovieSceneChannels", "ChannelRoll", "Roll"));
		MetaData[2].SortOrder = 0;
		MetaData[2].Color = FCommonChannelData::BlueChannelColor;
		MetaData[2].bCanCollapseToTrack = false;

		ExternalValues[0].OnGetExternalValue = ExtractChannelPitch;
		ExternalValues[1].OnGetExternalValue = ExtractChannelYaw;
		ExternalValues[2].OnGetExternalValue = ExtractChannelRoll;

		ExternalValues[0].OnGetCurrentValueAndWeight = GetChannelPitchValueAndWeight;
		ExternalValues[1].OnGetCurrentValueAndWeight = GetChannelYawValueAndWeight;
		ExternalValues[2].OnGetCurrentValueAndWeight = GetChannelRollValueAndWeight;
	}

	static FRotator GetPropertyValue(UObject& InObject, FTrackInstancePropertyBindings& Bindings)
	{
		return Bindings.GetCurrentValue<FRotator>(InObject);
	}

	static TOptional<float> ExtractChannelPitch(UObject& InObject, FTrackInstancePropertyBindings* Bindings)
	{
		return Bindings ? GetPropertyValue(InObject, *Bindings).Pitch : TOptional<float>();
	}
	static TOptional<float> ExtractChannelYaw(UObject& InObject, FTrackInstancePropertyBindings* Bindings)
	{
		return Bindings ? GetPropertyValue(InObject, *Bindings).Yaw : TOptional<float>();
	}
	static TOptional<float> ExtractChannelRoll(UObject& InObject, FTrackInstancePropertyBindings* Bindings)
	{
		return Bindings ? GetPropertyValue(InObject, *Bindings).Roll : TOptional<float>();
	}

	static void GetChannelPitchValueAndWeight(UObject* Object, UMovieSceneSection*  SectionToKey, FFrameNumber KeyTime, FFrameRate TickResolution, FMovieSceneRootEvaluationTemplateInstance& RootTemplate,
		float& OutValue, float OutWeight)
	{
		GetChannelValueAndWeight(0, Object, SectionToKey, KeyTime, TickResolution, RootTemplate, OutValue, OutWeight);
	}
	static void GetChannelYawValueAndWeight(UObject* Object, UMovieSceneSection*  SectionToKey, FFrameNumber KeyTime, FFrameRate TickResolution, FMovieSceneRootEvaluationTemplateInstance& RootTemplate,
		float& OutValue, float OutWeight)
	{
		GetChannelValueAndWeight(1, Object, SectionToKey, KeyTime, TickResolution, RootTemplate, OutValue, OutWeight);
	}
	static void GetChannelRollValueAndWeight(UObject* Object, UMovieSceneSection*  SectionToKey, FFrameNumber KeyTime, FFrameRate TickResolution, FMovieSceneRootEvaluationTemplateInstance& RootTemplate,
		float& OutValue, float OutWeight)
	{
		GetChannelValueAndWeight(2, Object, SectionToKey, KeyTime, TickResolution, RootTemplate, OutValue, OutWeight);
	}

	static void GetChannelValueAndWeight(int32 Index, UObject* Object, UMovieSceneSection*  SectionToKey, FFrameNumber KeyTime, FFrameRate TickResolution, FMovieSceneRootEvaluationTemplateInstance& RootTemplate,
		float& OutValue, float& OutWeight)
	{
		OutValue = 0.0f;
		OutWeight = 1.0f;

		UMovieSceneTrack* Track = SectionToKey->GetTypedOuter<UMovieSceneTrack>();

		if (Track)
		{
			FMovieSceneEvaluationTrack EvalTrack = CastChecked<UMovieSceneRotatorTrack>(Track)->GenerateTrackTemplate(Track);
			FMovieSceneInterrogationData InterrogationData;
			RootTemplate.CopyActuators(InterrogationData.GetAccumulator());

			FMovieSceneContext Context(FMovieSceneEvaluationRange(KeyTime, TickResolution));
			EvalTrack.Interrogate(Context, InterrogationData, Object);

			FRotator Val(0.0f, 0.0f, 0.0f);
			for (const FRotator& InRotator : InterrogationData.Iterate<FRotator>(FMovieSceneRotatorSectionTemplate::GetRotatorInterrogationKey()))
			{
				Val = InRotator;
				break;
			}
			
			switch (Index)
			{
			case 0:
				OutValue = Val.Pitch;
				break;
			case 1:
				OutValue = Val.Yaw;
				break;
			case 2:
				OutValue = Val.Roll;
				break;
			default:
				break;
			}
			
		}
		OutWeight = MovieSceneHelpers::CalculateWeightForBlending(SectionToKey, KeyTime);
	}

	FMovieSceneChannelMetaData      MetaData[3];
	TMovieSceneExternalValue<float> ExternalValues[3];
};
#endif

/* FMovieSceneRotatorKeyStruct interface
 *****************************************************************************/

void FMovieSceneRotatorKeyStruct::PropagateChanges(const FPropertyChangedEvent& ChangeEvent)
{
	KeyStructInterop.Apply(Time);
}

/* UMovieSceneRotatorSection structors
 *****************************************************************************/

UMovieSceneRotatorSection::UMovieSceneRotatorSection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	EvalOptions.EnableAndSetCompletionMode
		(GetLinkerCustomVersion(FSequencerObjectVersion::GUID) < FSequencerObjectVersion::WhenFinishedDefaultsToRestoreState ? 
			EMovieSceneCompletionMode::KeepState : 
			GetLinkerCustomVersion(FSequencerObjectVersion::GUID) < FSequencerObjectVersion::WhenFinishedDefaultsToProjectDefault ? 
			EMovieSceneCompletionMode::RestoreState : 
			EMovieSceneCompletionMode::ProjectDefault);
	BlendType = EMovieSceneBlendType::Absolute;
	bSupportsInfiniteRange = true;

	FMovieSceneChannelProxyData Channels;

#if WITH_EDITOR

	static FRotatorSectionEditorData EditorData;
	Channels.Add(PitchCurve,   EditorData.MetaData[0], EditorData.ExternalValues[0]);
	Channels.Add(YawCurve, EditorData.MetaData[1], EditorData.ExternalValues[1]);
	Channels.Add(RollCurve,  EditorData.MetaData[2], EditorData.ExternalValues[2]);

#else

	Channels.Add(PitchCurve);
	Channels.Add(YawCurve);
	Channels.Add(RollCurve);

#endif

	ChannelProxy = MakeShared<FMovieSceneChannelProxy>(MoveTemp(Channels));
}

TSharedPtr<FStructOnScope> UMovieSceneRotatorSection::GetKeyStruct(TArrayView<const FKeyHandle> KeyHandles)
{
	TSharedRef<FStructOnScope> KeyStruct = MakeShareable(new FStructOnScope(FMovieSceneRotatorKeyStruct::StaticStruct()));
	auto Struct = (FMovieSceneRotatorKeyStruct*)KeyStruct->GetStructMemory();

	Struct->KeyStructInterop.Add(FMovieSceneChannelValueHelper(ChannelProxy->MakeHandle<FMovieSceneFloatChannel>(0), &Struct->Rotator.Pitch, KeyHandles));
	Struct->KeyStructInterop.Add(FMovieSceneChannelValueHelper(ChannelProxy->MakeHandle<FMovieSceneFloatChannel>(1), &Struct->Rotator.Yaw, KeyHandles));
	Struct->KeyStructInterop.Add(FMovieSceneChannelValueHelper(ChannelProxy->MakeHandle<FMovieSceneFloatChannel>(2), &Struct->Rotator.Roll, KeyHandles));

	Struct->KeyStructInterop.SetStartingValues();
	Struct->Time = Struct->KeyStructInterop.GetUnifiedKeyTime().Get(0);

	return KeyStruct;
}
