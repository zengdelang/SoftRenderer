#pragma once

#include "CoreMinimal.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Evaluation/MovieSceneEvalTemplate.h"
#include "Evaluation/MovieScenePropertyTemplate.h"
#include "MovieSceneRotatorTemplate.generated.h"

class UMovieSceneRotatorSection;
class UMovieSceneRotatorTrack;

namespace UE
{
namespace MovieScene
{
	inline void MultiChannelFromData(const FRotator& In, TMultiChannelValue<float, 3>& Out)
	{
		Out = { In.Pitch, In.Yaw, In.Roll};
	}
	
	inline void ResolveChannelsToData(const TMultiChannelValue<float, 3>& In, FRotator& Out)
	{
		Out = FRotator(In[0], In[1], In[2]);
	}
} // namespace MovieScene
} // namespace UE

template<> UGUI_API FMovieSceneAnimTypeID GetBlendingDataType<FRotator>();

template<> struct TBlendableTokenTraits<FRotator>   { typedef UE::MovieScene::TMaskedBlendable<float, 3>    WorkingDataType; };

USTRUCT()
struct UGUI_API FMovieSceneRotatorSectionTemplate : public FMovieScenePropertySectionTemplate
{
	GENERATED_BODY()
	
	FMovieSceneRotatorSectionTemplate() : BlendType((EMovieSceneBlendType)0) {}
	FMovieSceneRotatorSectionTemplate(const UMovieSceneRotatorSection& Section, const UMovieSceneRotatorTrack& Track);

	UPROPERTY()
	FMovieSceneFloatChannel Curves[3];

	UPROPERTY()
	EMovieSceneBlendType BlendType;

public:
	const static FMovieSceneInterrogationKey GetRotatorInterrogationKey();

private:
	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;
	virtual void Interrogate(const FMovieSceneContext& Context, FMovieSceneInterrogationData& Container, UObject* BindingOverride) const override;

};
