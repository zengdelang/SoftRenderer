#pragma once

#include "CoreMinimal.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Sections/MovieSceneActorReferenceSection.h"
#include "UISequenceCommonDefinitions.generated.h"

class UUISequenceComponent;

struct IUISequenceEvaluationData
{
	virtual ~IUISequenceEvaluationData(){}

	virtual void Update(const float PlayingTime, UObject* ObjectBinding, UUISequenceComponent* SequenceComp) = 0;
	virtual void Stop() = 0;
	virtual void OnEndValuation() = 0;
};

UENUM(BlueprintType)
enum class EUISequenceTrackType : uint8
{
	Bool UMETA(DisplayName = Bool),
	Float UMETA(DisplayName = Float),
	Int UMETA(DisplayName = Int),
	Vector UMETA(DisplayName = Vector),
	Transform UMETA(DisplayName = Vector),
	MaterialParameter UMETA(DisplayName = Vector),
	Byte UMETA(DisplayName = Byte),
	AudioEvent UMETA(DisplayName = AudioEvent),
};

UENUM(BlueprintType)
enum class EUISequenceVectorSectionType : uint8
{
	Vector2 UMETA(DisplayName = Vector2),
	Vector3 UMETA(DisplayName = Vector3),
	Vector4 UMETA(DisplayName = Vector4),
	LinearColor UMETA(DisplayName = LinearColor),
	Rotator UMETA(DisplayName = Rotator),
};

USTRUCT(BlueprintType)
struct FUISequenceTrack
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FName TrackName;

	UPROPERTY()
	EUISequenceTrackType TrackType;

	UPROPERTY()
	uint16 SectionIndex;
};

USTRUCT(BlueprintType)
struct FUISequenceBinding
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	FName BindingName;

	UPROPERTY()
	TArray<FUISequenceTrack> Tracks;

	UPROPERTY()
	uint8 bMaterTrack : 1;

public:
	FUISequenceBinding() : bMaterTrack(false)
	{
	}
};

USTRUCT(BlueprintType)
struct FUISequenceBoolCurve
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(meta=(KeyTimes))
	TArray<float> Times;

	UPROPERTY(meta=(KeyValues))
	TArray<bool> Values;

	UPROPERTY()
	uint8 DefaultValue : 1;

	UPROPERTY()
	uint8 bHasDefaultValue : 1;
	
public:
	FUISequenceBoolCurve(): DefaultValue(false), bHasDefaultValue(false)
	{
	}
};

USTRUCT()
struct FUISequenceFloatValue
{
	GENERATED_BODY()

	FUISequenceFloatValue()
		: Value(0.f), InterpMode(RCIM_Cubic), TangentMode(RCTM_Auto)
	{}

	explicit FUISequenceFloatValue(float InValue)
		: Value(InValue), InterpMode(RCIM_Cubic), TangentMode(RCTM_Auto)
	{}
	
	UPROPERTY()
	float Value;

	UPROPERTY()
	FMovieSceneTangentData Tangent;

	UPROPERTY()
	TEnumAsByte<ERichCurveInterpMode> InterpMode;

	UPROPERTY()
	TEnumAsByte<ERichCurveTangentMode> TangentMode;
	
};

USTRUCT(BlueprintType)
struct FUISequenceFloatCurve
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	TArray<float> Times;

	UPROPERTY()
	TArray<FUISequenceFloatValue> Values;

	/** Pre-infinity extrapolation state */
	UPROPERTY()
	TEnumAsByte<ERichCurveExtrapolation> PreInfinityExtrap;

	/** Post-infinity extrapolation state */
	UPROPERTY()
	TEnumAsByte<ERichCurveExtrapolation> PostInfinityExtrap;

	UPROPERTY()
	float DefaultValue;

	UPROPERTY()
	uint8 bHasDefaultValue : 1;
	
public:
	FUISequenceFloatCurve(): PreInfinityExtrap(), PostInfinityExtrap(), DefaultValue(0), bHasDefaultValue(0)
	{
	}
};

USTRUCT(BlueprintType)
struct FUISequenceIntCurve
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	TArray<float> Times;

	UPROPERTY()
	TArray<int32> Values;

	UPROPERTY()
	int32 DefaultValue;

	UPROPERTY()
	uint8 bHasDefaultValue : 1;
	
public:
	FUISequenceIntCurve(): DefaultValue(0), bHasDefaultValue(0)
	{
	}
};

USTRUCT(BlueprintType)
struct FUISequenceVectorCurve
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	EUISequenceVectorSectionType SectionType;
	
	UPROPERTY()
	uint16 X;

	UPROPERTY()
	uint16 Y;

	UPROPERTY()
	uint16 Z;

	UPROPERTY()
	uint16 W;
};

USTRUCT(BlueprintType)
struct FUISequenceTransformCurve
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	uint16 LocationX;

	UPROPERTY()
	uint16 LocationY;

	UPROPERTY()
	uint16 LocationZ;

	UPROPERTY()
	uint16 RotationX;

	UPROPERTY()
	uint16 RotationY;

	UPROPERTY()
	uint16 RotationZ;

	UPROPERTY()
	uint16 ScaleX;

	UPROPERTY()
	uint16 ScaleY;

	UPROPERTY()
	uint16 ScaleZ;

public:
	FUISequenceTransformCurve()
		: LocationX(UINT16_MAX)
		, LocationY(UINT16_MAX)
		, LocationZ(UINT16_MAX)
		, RotationX(UINT16_MAX)
		, RotationY(UINT16_MAX)
		, RotationZ(UINT16_MAX)
		, ScaleX(UINT16_MAX)
		, ScaleY(UINT16_MAX)
		, ScaleZ(UINT16_MAX)
	{
	}
};

USTRUCT()
struct FUIMaterialParameterValue
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	FName ParameterName;
	
	UPROPERTY()
	uint16 SectionIndex;
};

USTRUCT()
struct FUIMaterialParameterCurve
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	int32 MaterialIndex;
	
	UPROPERTY()
	TArray<FUIMaterialParameterValue> Scalars;
	
	UPROPERTY()
	TArray<FUIMaterialParameterValue> Bools;

	UPROPERTY()
	TArray<FUIMaterialParameterValue> Vector2Ds;

	UPROPERTY()
	TArray<FUIMaterialParameterValue> Vectors;

	UPROPERTY()
	TArray<FUIMaterialParameterValue> Colors;

	UPROPERTY()
	TArray<FUIMaterialParameterValue> Transforms;
};

USTRUCT()
struct FUISequenceAudioSection
{
	GENERATED_BODY()
	
public:
	/** The sound cue or wave that this section plays */
	UPROPERTY()
	USoundBase* Sound;

	/** The attenuation settings to use. */
	UPROPERTY()
	class USoundAttenuation* AttenuationSettings;
	
	UPROPERTY()
	uint16 VolumeCurveIndex;

	UPROPERTY()
	uint16 PitchMultiplierCurveIndex;
	
	TWeakPtr<IUISequenceEvaluationData> EvaluationDataPtr;

	/** The offset into the beginning of the audio clip */
	UPROPERTY()
	float StartFrameOffset;
	
	UPROPERTY()
	float ClipStartTime;

	UPROPERTY()
	float ClipEndTime;
	
	UPROPERTY()
	int32 SectionIndex; 
	
	/* Allow looping if the section length is greater than the sound duration */
	UPROPERTY()
	uint8 bLooping : 1;

public:
	FUISequenceAudioSection() : Sound(nullptr), AttenuationSettings(nullptr), VolumeCurveIndex(0),
	                            PitchMultiplierCurveIndex(0), StartFrameOffset(0),
	                            ClipStartTime(0), ClipEndTime(0),
	                            SectionIndex(0),
	                            bLooping(false)
	{
	}
};

USTRUCT()
struct FUISequenceAudioCurve
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TArray<FUISequenceAudioSection> AudioSections;

	UPROPERTY()
	uint8 bMasterTrack: 1;
	
};
