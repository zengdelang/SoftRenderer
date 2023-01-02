#include "Sequence/Evaluation/MovieSceneRotatorTemplate.h"
#include "Evaluation/MovieSceneEvaluation.h"
#include "IMovieScenePlayer.h"
#include "Evaluation/MovieScenePropertyTemplate.h"
#include "Evaluation/Blending/MovieSceneMultiChannelBlending.h"
#include "Sequence/Sections/MovieSceneRotatorSection.h"
#include "Sequence/Tracks/MovieSceneRotatorTrack.h"

#if (ENGINE_MINOR_VERSION >= 27 && ENGINE_MAJOR_VERSION <= 4) || (ENGINE_MAJOR_VERSION > 4)  
#include "Evaluation/PreAnimatedState/MovieSceneRestoreStateParams.h"
#endif

template<> FMovieSceneAnimTypeID GetBlendingDataType<FRotator>()   { static FMovieSceneAnimTypeID TypeId = FMovieSceneAnimTypeID::Unique(); return TypeId; }

struct FRotatorToken
{
	FRotator RotatorValue;

	FRotatorToken() {}
	FRotatorToken(FRotator InRotatorValue) : RotatorValue(InRotatorValue){}

	void Apply(UObject& Object, FTrackInstancePropertyBindings& Bindings)
	{
		const FRotator NewRotator(RotatorValue);
		Bindings.CallFunction<FRotator>(Object, NewRotator);
	}

	static FRotatorToken Get(const UObject& InObject, FTrackInstancePropertyBindings& Bindings)
	{
		FRotatorToken Token;
		Token.RotatorValue = Bindings.GetCurrentValue<FRotator>(InObject);				
		return Token;
	}

};

struct FRotatorTrackPreAnimatedState : IMovieScenePreAnimatedToken
{
	FRotatorToken Token;
	FTrackInstancePropertyBindings Bindings;

	FRotatorTrackPreAnimatedState(FRotatorToken InToken, const FTrackInstancePropertyBindings& InBindings)
		: Token(InToken)
		, Bindings(InBindings)
	{
	}

	virtual void RestoreState(UObject& Object, IMovieScenePlayer& Player) override
	{
		Token.Apply(Object, Bindings);
	}

#if (ENGINE_MINOR_VERSION >= 27 && ENGINE_MAJOR_VERSION <= 4) || (ENGINE_MAJOR_VERSION > 4)  
	virtual void RestoreState(UObject& Object, const UE::MovieScene::FRestoreStateParams& Params) override
	{
		Token.Apply(Object, Bindings);
	}
#endif
};

struct FRotatorTokenProducer : IMovieScenePreAnimatedTokenProducer
{
	FTrackInstancePropertyBindings& PropertyBindings;
	FRotatorTokenProducer(FTrackInstancePropertyBindings& InPropertyBindings) : PropertyBindings(InPropertyBindings) {}

	virtual IMovieScenePreAnimatedTokenPtr CacheExistingState(UObject& Object) const override
	{
		return FRotatorTrackPreAnimatedState(FRotatorToken::Get(Object, PropertyBindings), PropertyBindings);
	}
};

struct FRotatorTokenActuator : TMovieSceneBlendingActuator<FRotator>
{
	PropertyTemplate::FSectionData PropertyData;
	FRotatorTokenActuator(const PropertyTemplate::FSectionData& InPropertyData)
		: TMovieSceneBlendingActuator<FRotator>(FMovieSceneBlendingActuatorID(InPropertyData.PropertyID))
		, PropertyData(InPropertyData)
	{}

	virtual FRotator RetrieveCurrentValue(UObject* InObject, IMovieScenePlayer* Player) const override
	{
		return FRotatorToken::Get(*InObject, *PropertyData.PropertyBindings).RotatorValue;
	}

	virtual void Actuate(UObject* InObject, const FRotator& InFinalValue, const TBlendableTokenStack<FRotator>& OriginalStack, const FMovieSceneContext& Context, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override
	{
		ensureMsgf(InObject, TEXT("Attempting to evaluate a Rotator track with a null object."));

		if (InObject != nullptr)
		{
			FTrackInstancePropertyBindings& PropertyBindings = *PropertyData.PropertyBindings;

			OriginalStack.SavePreAnimatedState(Player, *InObject, PropertyData.PropertyID, FRotatorTokenProducer(PropertyBindings));

			// Apply a token
			FRotatorToken(InFinalValue).Apply(*InObject, PropertyBindings);
		}
	}
	
	virtual void Actuate(FMovieSceneInterrogationData& InterrogationData, typename TCallTraits<FRotator>::ParamType InValue, const TBlendableTokenStack<FRotator>& OriginalStack, const FMovieSceneContext& Context) const override
	{
		FRotator Value = InValue;
		InterrogationData.Add(Value, FMovieSceneRotatorSectionTemplate::GetRotatorInterrogationKey());
	};

};

FMovieSceneRotatorSectionTemplate::FMovieSceneRotatorSectionTemplate(const UMovieSceneRotatorSection& Section, const UMovieSceneRotatorTrack& Track)
	: FMovieScenePropertySectionTemplate(Track.GetPropertyName(), Track.GetPropertyPath().ToString())
	, BlendType(Section.GetBlendType().Get())
{
	Curves[0] = Section.GetPitchChannel();
	Curves[1] = Section.GetYawChannel();
	Curves[2] = Section.GetRollChannel();
}

const FMovieSceneInterrogationKey FMovieSceneRotatorSectionTemplate::GetRotatorInterrogationKey()
{
	const static FMovieSceneAnimTypeID TypeID = FMovieSceneAnimTypeID::Unique();
	return TypeID;
}

void FMovieSceneRotatorSectionTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
	const FFrameTime Time = Context.GetTime();
	UE::MovieScene::TMultiChannelValue<float, 3> AnimationData;

	for (uint8 Index = 0; Index < 3; ++Index)
	{
		float ChannelValue = 0.f;
		if (Curves[Index].Evaluate(Time, ChannelValue))
		{
			AnimationData.Set(Index, ChannelValue);
		}
	}

	// Only blend the token if at least one of the channels was animated
	if (!AnimationData.IsEmpty())
	{
		// Actuator type ID for this property
		FMovieSceneAnimTypeID UniquePropertyID = GetPropertyTypeID();
		FMovieSceneBlendingActuatorID ActuatorTypeID = FMovieSceneBlendingActuatorID(UniquePropertyID);
		if (!ExecutionTokens.GetBlendingAccumulator().FindActuator<FRotator>(ActuatorTypeID))
		{
			PropertyTemplate::FSectionData SectionData;
			SectionData.Initialize(PropertyData.PropertyName, PropertyData.PropertyPath);

			ExecutionTokens.GetBlendingAccumulator().DefineActuator(ActuatorTypeID, MakeShared<FRotatorTokenActuator>(SectionData));
		}

		const float Weight = EvaluateEasing(Time);
		ExecutionTokens.BlendToken(ActuatorTypeID, TBlendableToken<FRotator>(AnimationData, BlendType, Weight));
	}
}

void FMovieSceneRotatorSectionTemplate::Interrogate(const FMovieSceneContext& Context, FMovieSceneInterrogationData& Container, UObject* BindingOverride) const
{
	const FFrameTime Time = Context.GetTime();
	UE::MovieScene::TMultiChannelValue<float, 3> AnimationData;

	for (uint8 Index = 0; Index < 3; ++Index)
	{
		float ChannelValue = 0.f;
		if (Curves[Index].Evaluate(Time, ChannelValue))
		{
			AnimationData.Set(Index, ChannelValue);
		}
	}

	FMovieSceneAnimTypeID TypeID = GetPropertyTypeID();
	static FMovieSceneBlendingActuatorID ActuatorTypeID(TypeID);
	if (!Container.GetAccumulator().FindActuator<FRotator>(ActuatorTypeID))
	{
		PropertyTemplate::FSectionData SectionData;
		SectionData.Initialize(PropertyData.PropertyName, PropertyData.PropertyPath);
		Container.GetAccumulator().DefineActuator(ActuatorTypeID, MakeShared<TPropertyActuator<FRotator>>(SectionData));
	}

	if (!AnimationData.IsEmpty())
	{
		// Add the blendable to the accumulator
		float Weight = EvaluateEasing(Context.GetTime());
		Container.GetAccumulator().BlendToken(FMovieSceneEvaluationOperand(), ActuatorTypeID, FMovieSceneEvaluationScope(), Context, TBlendableToken<FRotator>(AnimationData, BlendType, Weight));
	}
}