#pragma once

#include "CoreMinimal.h"
#include "KeyPropertyParams.h"
#include "MovieSceneTools/Public/PropertyTrackEditor.h"
#include "Sequence/Tracks/MovieSceneRotatorTrack.h"
#include "ISequencer.h"
#include "ISequencerTrackEditor.h"

/**
 * A property track editor for Rotator.
 */
class FRotatorPropertyTrackEditor
	: public FPropertyTrackEditor<UMovieSceneRotatorTrack>
{
public:
	/**
	 * Constructor.
	 *
	 * @param InSequencer The sequencer instance to be used by this tool.
	 */
	FRotatorPropertyTrackEditor(TSharedRef<ISequencer> InSequencer)
		: FPropertyTrackEditor(InSequencer, GetAnimatedPropertyTypes())
	{
	}

	/**
	 * Retrieve a list of all property types that this track editor animates
	 */
	static TArray<FAnimatedPropertyKey, TInlineAllocator<1>> GetAnimatedPropertyTypes()
	{
		return TArray<FAnimatedPropertyKey, TInlineAllocator<1>>({
			FAnimatedPropertyKey::FromStructType(NAME_Rotator),
		});
	}

	/**
	 * Creates an instance of this class (called by a sequence).
	 *
	 * @param OwningSequencer The sequencer instance to be used by this tool.
	 * @return The new instance of this class.
	 */
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer);

protected:
	virtual void GenerateKeysFromPropertyChanged(const FPropertyChangedParams& PropertyChangedParams, UMovieSceneSection* SectionToKey, FGeneratedTrackKeys& OutGeneratedKeys) override;
	virtual bool ModifyGeneratedKeysByCurrentAndWeight(UObject* Object, UMovieSceneTrack *Track, UMovieSceneSection* SectionToKey, FFrameNumber Time, FGeneratedTrackKeys& GeneratedTotalKeys, float Weight) const override;

private:
	static FName PitchName;
	static FName YawName;
	static FName RollName;

};
