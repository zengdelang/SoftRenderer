#include "Sequence/Tracks/MovieSceneUIMaterialTrack.h"
#include "MovieSceneCommonHelpers.h"
#include "Evaluation/MovieSceneEvaluationTrack.h"
#include "Sequence/Evaluation/MovieSceneUIParameterTemplate.h"

UMovieSceneUIMaterialTrack::UMovieSceneUIMaterialTrack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	TrackTint = FColor(64,192,64,65);
#endif
}

bool UMovieSceneUIMaterialTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UMovieSceneParameterSection::StaticClass();
}

UMovieSceneSection* UMovieSceneUIMaterialTrack::CreateNewSection()
{
	return NewObject<UMovieSceneParameterSection>(this, NAME_None, RF_Transactional);
}

void UMovieSceneUIMaterialTrack::RemoveAllAnimationData()
{
	Sections.Empty();
}

bool UMovieSceneUIMaterialTrack::HasSection(const UMovieSceneSection& Section) const
{
	return Sections.Contains(&Section);
}

void UMovieSceneUIMaterialTrack::AddSection(UMovieSceneSection& Section)
{
	Sections.Add(&Section);
}

void UMovieSceneUIMaterialTrack::RemoveSection(UMovieSceneSection& Section)
{
	Sections.Remove(&Section);
}

void UMovieSceneUIMaterialTrack::RemoveSectionAt(int32 SectionIndex)
{
	Sections.RemoveAt(SectionIndex);
}

bool UMovieSceneUIMaterialTrack::IsEmpty() const
{
	return Sections.Num() == 0;
}

const TArray<UMovieSceneSection*>& UMovieSceneUIMaterialTrack::GetAllSections() const
{
	return Sections;
}

void UMovieSceneUIMaterialTrack::AddScalarParameterKey(FName ParameterName, FFrameNumber Time, float Value)
{
	UMovieSceneParameterSection* NearestSection = Cast<UMovieSceneParameterSection>(MovieSceneHelpers::FindNearestSectionAtTime(Sections, Time));
	if (NearestSection == nullptr)
	{
		NearestSection = Cast<UMovieSceneParameterSection>(CreateNewSection());

		UMovieScene* MovieScene = GetTypedOuter<UMovieScene>();
		check(MovieScene);

		NearestSection->SetRange(MovieScene->GetPlaybackRange());
		Sections.Add(NearestSection);
	}
	if (NearestSection->TryModify())
	{
		NearestSection->AddScalarParameterKey(ParameterName, Time, Value);
	}
}

void UMovieSceneUIMaterialTrack::AddColorParameterKey(FName ParameterName, FFrameNumber Time, FLinearColor Value)
{
	UMovieSceneParameterSection* NearestSection = Cast<UMovieSceneParameterSection>(MovieSceneHelpers::FindNearestSectionAtTime(Sections, Time));
	if (NearestSection == nullptr)
	{
		NearestSection = Cast<UMovieSceneParameterSection>(CreateNewSection());

		UMovieScene* MovieScene = GetTypedOuter<UMovieScene>();
		check(MovieScene);

		NearestSection->SetRange(MovieScene->GetPlaybackRange());

		Sections.Add(NearestSection);
	}
	if (NearestSection->TryModify())
	{
		NearestSection->AddColorParameterKey(ParameterName, Time, Value);
	}
}

UMovieSceneUIComponentMaterialTrack::UMovieSceneUIComponentMaterialTrack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

FMovieSceneEvalTemplatePtr UMovieSceneUIComponentMaterialTrack::CreateTemplateForSection(const UMovieSceneSection& InSection) const
{
	return FMovieSceneUIComponentMaterialSectionTemplate(*CastChecked<UMovieSceneParameterSection>(&InSection), *this);
}

void UMovieSceneUIComponentMaterialTrack::PostCompile(FMovieSceneEvaluationTrack& OutTrack, const FMovieSceneTrackCompilerArgs& Args) const
{
	OutTrack.SetEvaluationPriority(EvaluationPriority);
}

#if WITH_EDITORONLY_DATA

FText UMovieSceneUIComponentMaterialTrack::GetDefaultDisplayName() const
{
	return FText::FromString(FString::Printf(TEXT("Material Element %i"), MaterialIndex));
}

#endif
