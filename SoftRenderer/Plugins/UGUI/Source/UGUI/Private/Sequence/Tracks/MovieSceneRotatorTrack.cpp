#include "Sequence/Tracks/MovieSceneRotatorTrack.h"
#include "Evaluation/MovieSceneEvalTemplate.h"
#include "Sequence/Evaluation/MovieSceneRotatorTemplate.h"
#include "Sequence/Sections/MovieSceneRotatorSection.h"

UMovieSceneRotatorTrack::UMovieSceneRotatorTrack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedBlendTypes = FMovieSceneBlendTypeField::All();
}

bool UMovieSceneRotatorTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UMovieSceneRotatorSection::StaticClass();
}

UMovieSceneSection* UMovieSceneRotatorTrack::CreateNewSection()
{
	return NewObject<UMovieSceneRotatorSection>(this, NAME_None, RF_Transactional);
}

FMovieSceneEvalTemplatePtr UMovieSceneRotatorTrack::CreateTemplateForSection(const UMovieSceneSection& Section) const
{
	return FMovieSceneRotatorSectionTemplate(*CastChecked<const UMovieSceneRotatorSection>(&Section), *this);
}
