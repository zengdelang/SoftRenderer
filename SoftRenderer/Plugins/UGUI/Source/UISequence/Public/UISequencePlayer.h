#pragma once

#include "IMovieScenePlayer.h"
#include "UISequence.h"
#include "MovieSceneSequencePlayer.h"
#include "UISequencePlayer.generated.h"

/**
 * UUISequencePlayer is used to actually "play" an UI sequence asset at runtime.
 */
UCLASS(BlueprintType)
class UISEQUENCE_API UUISequencePlayer : public UMovieSceneSequencePlayer
{
	GENERATED_BODY()

protected:
	//~ IMovieScenePlayer interface
	virtual UObject* GetPlaybackContext() const override;
	virtual TArray<UObject*> GetEventContexts() const override;
	
};
