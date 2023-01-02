#pragma once

#include "CoreMinimal.h"
#include "Compilation/IMovieSceneTrackTemplateProducer.h"
#include "Tracks/MovieScenePropertyTrack.h"
#include "MovieSceneRotatorTrack.generated.h"

struct FMovieSceneInterrogationKey;

UCLASS(MinimalAPI)
class UMovieSceneRotatorTrack
	: public UMovieScenePropertyTrack, public IMovieSceneTrackTemplateProducer
{
	GENERATED_UCLASS_BODY()

public:
	// UMovieSceneTrack interface
	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override;
	virtual UMovieSceneSection* CreateNewSection() override;
	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& Section) const override;
	
};

