#include "UISequence.h"
#include "MovieScene.h"
#include "MovieSceneCommonHelpers.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "UISequenceComponent.h"
#include "Engine/LevelScriptActor.h"

#if WITH_EDITOR
#include "Tracks/MovieSceneAudioTrack.h"
#endif

#if WITH_EDITOR
UUISequence::FOnInitialize UUISequence::OnInitializeSequenceEvent;
#endif

static TAutoConsoleVariable<int32> CVarDefaultEvaluationType(
	TEXT("UISequence.DefaultEvaluationType"),
	0,
	TEXT("0: Playback locked to playback frames\n1: Unlocked playback with sub frame interpolation"),
	ECVF_Default);

static TAutoConsoleVariable<FString> CVarDefaultTickResolution(
	TEXT("UISequence.DefaultTickResolution"),
	TEXT("24000fps"),
	TEXT("Specifies default a tick resolution for newly created level sequences. Examples: 30 fps, 120/1 (120 fps), 30000/1001 (29.97), 0.01s (10ms)."),
	ECVF_Default);

static TAutoConsoleVariable<FString> CVarDefaultDisplayRate(
	TEXT("UISequence.DefaultDisplayRate"),
	TEXT("30fps"),
	TEXT("Specifies default a display frame rate for newly created level sequences; also defines frame locked frame rate where sequences are set to be frame locked. Examples: 30 fps, 120/1 (120 fps), 30000/1001 (29.97), 0.01s (10ms)."),
	ECVF_Default);

UUISequence::UUISequence(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
#if WITH_EDITORONLY_DATA
	, MovieScene(nullptr)
	, bHasBeenInitialized(false)
#endif
{
	bParentContextsAreSignificant = true;

#if WITH_EDITORONLY_DATA
	MovieScene = ObjectInitializer.CreateEditorOnlyDefaultSubobject<UMovieScene>(this, "MovieScene");
	if (MovieScene)
	{
		MovieScene->SetFlags(RF_Transactional);
	}
#endif
}

bool UUISequence::IsEditable() const
{
	const UObject* Template = GetArchetype();

	if (Template == GetDefault<UUISequence>())
	{
		return false;
	}

	return !Template || Template->GetTypedOuter<UUISequenceComponent>() == GetDefault<UUISequenceComponent>();
}

UBlueprint* UUISequence::GetParentBlueprint() const
{
	if (const UBlueprintGeneratedClass* GeneratedClass = GetTypedOuter<UBlueprintGeneratedClass>())
	{
		return Cast<UBlueprint>(GeneratedClass->ClassGeneratedBy);
	}
	return nullptr;
}

void UUISequence::PostInitProperties()
{
#if WITH_EDITOR && WITH_EDITORONLY_DATA
	// We do not run the default initialization for actor sequences that are CDOs, or that are going to be loaded (since they will have already been initialized in that case)
	constexpr EObjectFlags ExcludeFlags = RF_ClassDefaultObject | RF_NeedLoad | RF_NeedPostLoad | RF_NeedPostLoadSubobjects | RF_WasLoaded;

	const UActorComponent* OwnerComponent = Cast<UActorComponent>(GetOuter());
	if (!bHasBeenInitialized && !HasAnyFlags(ExcludeFlags) && OwnerComponent && !OwnerComponent->HasAnyFlags(ExcludeFlags))
	{
		const AActor* Actor = Cast<AActor>(OwnerComponent->GetOuter());

		const FGuid BindingID = MovieScene->AddPossessable(Actor ? Actor->GetActorLabel() : TEXT("Owner"), Actor ? Actor->GetClass() : AActor::StaticClass());
		ObjectReferences.CreateBinding(BindingID, FUISequenceObjectReference::CreateForContextActor());

		const bool bFrameLocked = CVarDefaultEvaluationType.GetValueOnGameThread() != 0;

		MovieScene->SetEvaluationType( bFrameLocked ? EMovieSceneEvaluationType::FrameLocked : EMovieSceneEvaluationType::WithSubFrames );

		FFrameRate TickResolution(60000, 1);
		TryParseString(TickResolution, *CVarDefaultTickResolution.GetValueOnGameThread());
		MovieScene->SetTickResolutionDirectly(TickResolution);

		FFrameRate DisplayRate(30, 1);
		TryParseString(DisplayRate, *CVarDefaultDisplayRate.GetValueOnGameThread());
		MovieScene->SetDisplayRate(DisplayRate);

		OnInitializeSequenceEvent.Broadcast(this);
		bHasBeenInitialized = true;
	}
#endif

	Super::PostInitProperties();
}

void UUISequence::BindPossessableObject(const FGuid& ObjectId, UObject& PossessedObject, UObject* Context)
{
	AActor* ActorContext = Cast<AActor>(Context);

	if (UActorComponent* Component = Cast<UActorComponent>(&PossessedObject))
	{
		ObjectReferences.CreateBinding(ObjectId, FUISequenceObjectReference::CreateForComponent(Component));
	}
	else if (AActor* Actor = Cast<AActor>(&PossessedObject))
	{
		ObjectReferences.CreateBinding(ObjectId, FUISequenceObjectReference::CreateForActor(Actor, ActorContext));
	}
}

bool UUISequence::CanPossessObject(UObject& Object, UObject* InPlaybackContext) const
{
	if (InPlaybackContext == nullptr)
	{
		return false;
	}

	const AActor* ActorContext = CastChecked<AActor>(InPlaybackContext);

	if (const AActor* Actor = Cast<AActor>(&Object))
	{
		return Actor == InPlaybackContext || Actor->GetLevel() == ActorContext->GetLevel();
	}
	else if (const UActorComponent* Component = Cast<UActorComponent>(&Object))
	{
		return Component->GetOwner() ? Component->GetOwner()->GetLevel() == ActorContext->GetLevel() : false;
	}
	return false;
}

void UUISequence::LocateBoundObjects(const FGuid& ObjectId, UObject* Context, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const
{
	if (Context)
	{
		ObjectReferences.ResolveBinding(ObjectId, CastChecked<AActor>(Context), OutObjects);
	}
}

#if WITH_EDITORONLY_DATA

UMovieScene* UUISequence::GetMovieScene() const
{
	return MovieScene;
}

#endif

UObject* UUISequence::GetParentObject(UObject* Object) const
{
	if (const UActorComponent* Component = Cast<UActorComponent>(Object))
	{
		return Component->GetOwner();
	}

	return nullptr;
}

void UUISequence::UnbindPossessableObjects(const FGuid& ObjectId)
{
	ObjectReferences.RemoveBinding(ObjectId);
}

UObject* UUISequence::CreateDirectorInstance(IMovieScenePlayer& Player)
{
	AActor* Actor = CastChecked<AActor>(Player.GetPlaybackContext(), ECastCheckedType::NullAllowed);
	if (!Actor)
	{
		return nullptr;
	}

	// If this sequence is inside a blueprint, or its component's archetype is from a blueprint, we use the actor as the instance (which will be an instance of the blueprint itself)
	if (GetTypedOuter<UBlueprintGeneratedClass>() || GetTypedOuter<UUISequenceComponent>()->GetArchetype() != GetDefault<UUISequenceComponent>())
	{
		return Actor;
	}

	// Otherwise we use the level script actor as the instance
	return Actor->GetLevel()->GetLevelScriptActor();
}

#if WITH_EDITOR

FText UUISequence::GetDisplayName() const
{
	const UUISequenceComponent* Component = GetTypedOuter<UUISequenceComponent>();

	if (Component)
	{
		FString OwnerName;
		
		if (const UBlueprint* Blueprint = GetParentBlueprint())
		{
			OwnerName = Blueprint->GetName();
		}
		else if(const AActor* Owner = Component->GetOwner())
		{
			OwnerName = Owner->GetActorLabel();
		}

		return OwnerName.IsEmpty()
			? FText::FromName(Component->GetFName())
			: FText::Format(NSLOCTEXT("UISequence", "DisplayName", "{0} ({1})"), FText::FromName(Component->GetFName()), FText::FromString(OwnerName));
	}

	return UMovieSceneSequence::GetDisplayName();
}

ETrackSupport UUISequence::IsTrackSupported(TSubclassOf<UMovieSceneTrack> InTrackClass) const
{
	if (UMovieSceneAudioTrack::StaticClass() == InTrackClass)
		return ETrackSupport::Supported;
	
	return ETrackSupport::Default;
}

#endif
