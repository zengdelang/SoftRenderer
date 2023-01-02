#include "Sequence/TrackEditors/PropertyTrackEditors/RotatorPropertyTrackEditor.h"
#include "MovieSceneToolHelpers.h"
#include "Evaluation/MovieScenePropertyTemplate.h"
#include "Sequence/Evaluation/MovieSceneRotatorTemplate.h"

FName FRotatorPropertyTrackEditor::PitchName("Pitch");
FName FRotatorPropertyTrackEditor::YawName("Yaw");
FName FRotatorPropertyTrackEditor::RollName("Roll");

TSharedRef<ISequencerTrackEditor> FRotatorPropertyTrackEditor::CreateTrackEditor( TSharedRef<ISequencer> InSequencer )
{
	return MakeShareable(new FRotatorPropertyTrackEditor(InSequencer));
}

void FRotatorPropertyTrackEditor::GenerateKeysFromPropertyChanged( const FPropertyChangedParams& PropertyChangedParams, UMovieSceneSection* SectionToKey, FGeneratedTrackKeys& OutGeneratedKeys )
{
	const FStructProperty* StructProp = CastField<const FStructProperty>( PropertyChangedParams.PropertyPath.GetLeafMostProperty().Property.Get() );
	if (!StructProp)
	{
		return;
	}
	
	const FRotator RotatorValues = PropertyChangedParams.GetPropertyValue<FRotator>();

	const FPropertyPath StructPath = PropertyChangedParams.StructPathToKey;
	const FName ChannelName = StructPath.GetNumProperties() != 0 ? StructPath.GetLeafMostProperty().Property->GetFName() : NAME_None;

	const bool bKeyPitch = ChannelName == NAME_None || ChannelName == PitchName;
	const bool bKeyYaw = ChannelName == NAME_None || ChannelName == YawName;
	const bool bKeyRoll = ChannelName == NAME_None || ChannelName == RollName;

	OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneFloatChannel>(0, RotatorValues.Pitch, bKeyPitch));
	OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneFloatChannel>(1, RotatorValues.Yaw, bKeyYaw));
	OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneFloatChannel>(2, RotatorValues.Roll, bKeyRoll));
}

bool FRotatorPropertyTrackEditor::ModifyGeneratedKeysByCurrentAndWeight(UObject *Object, UMovieSceneTrack *Track, UMovieSceneSection* SectionToKey, FFrameNumber KeyTime, FGeneratedTrackKeys& GeneratedTotalKeys, float Weight) const
{
	IMovieSceneTrackTemplateProducer* TrackTemplateProducer = Cast<IMovieSceneTrackTemplateProducer>(Track);
	if (!TrackTemplateProducer)
	{
		return false;
	}

	FFrameRate TickResolution = GetSequencer()->GetFocusedTickResolution();

	UMovieSceneRotatorTrack* RotatorTrack = Cast<UMovieSceneRotatorTrack>(Track);
	FMovieSceneEvaluationTrack EvalTrack = TrackTemplateProducer->GenerateTrackTemplate(Track);

	if (RotatorTrack)
	{
		FMovieSceneInterrogationData InterrogationData;
		GetSequencer()->GetEvaluationTemplate().CopyActuators(InterrogationData.GetAccumulator());

		FMovieSceneContext Context(FMovieSceneEvaluationRange(KeyTime, GetSequencer()->GetFocusedTickResolution()));
		EvalTrack.Interrogate(Context, InterrogationData, Object);

		FRotator Val(0.0f, 0.0f, 0.0f);
		for (const FRotator& InRotator : InterrogationData.Iterate<FRotator>(FMovieSceneRotatorSectionTemplate::GetRotatorInterrogationKey()))
		{
			Val = InRotator;
			break;
		}
		FMovieSceneChannelProxy& Proxy = SectionToKey->GetChannelProxy();
		GeneratedTotalKeys[0]->ModifyByCurrentAndWeight(Proxy, KeyTime, (void *)&Val.Pitch, Weight);
		GeneratedTotalKeys[1]->ModifyByCurrentAndWeight(Proxy, KeyTime, (void *)&Val.Yaw, Weight);
		GeneratedTotalKeys[2]->ModifyByCurrentAndWeight(Proxy, KeyTime, (void *)&Val.Roll, Weight);
		return true;
	}
	return false;
}