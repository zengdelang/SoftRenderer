#pragma once

#include "CoreMinimal.h"
#include "Curves/KeyHandle.h"
#include "MovieSceneSection.h"
#include "MovieSceneKeyStruct.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "MovieSceneRotatorSection.generated.h"

struct FPropertyChangedEvent;

/**
* Proxy structure for rotator section key data.
*/
USTRUCT()
struct FMovieSceneRotatorKeyStruct
	: public FMovieSceneKeyStruct
{
	GENERATED_BODY()

	/** The key's rotator value. */
	UPROPERTY(EditAnywhere, Category = Key)
	FRotator Rotator;

	/** The key's time. */
	UPROPERTY(EditAnywhere, Category = Key)
	FFrameNumber Time;

	FMovieSceneKeyStructHelper KeyStructInterop;

	virtual void PropagateChanges(const FPropertyChangedEvent& ChangeEvent) override;
};
template<> struct TStructOpsTypeTraits<FMovieSceneRotatorKeyStruct> : public TStructOpsTypeTraitsBase2<FMovieSceneRotatorKeyStruct> { enum { WithCopy = false }; };

UCLASS()
class UGUI_API UMovieSceneRotatorSection
	: public UMovieSceneSection
{
	GENERATED_UCLASS_BODY()

public:
	const FMovieSceneFloatChannel& GetPitchChannel() const { return PitchCurve; }

	const FMovieSceneFloatChannel& GetYawChannel() const { return YawCurve; }

	const FMovieSceneFloatChannel& GetRollChannel() const { return RollCurve; }
	
protected:
	virtual TSharedPtr<FStructOnScope> GetKeyStruct(TArrayView<const FKeyHandle> KeyHandles) override;

private:
	/** Pitch curve data */
	UPROPERTY()
	FMovieSceneFloatChannel PitchCurve;

	/** Yaw curve data */
	UPROPERTY()
	FMovieSceneFloatChannel YawCurve;

	/** Roll curve data */
	UPROPERTY()
	FMovieSceneFloatChannel RollCurve;

};
