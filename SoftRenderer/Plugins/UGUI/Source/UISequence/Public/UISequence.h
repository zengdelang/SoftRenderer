#pragma once

#include "MovieSceneSequence.h"
#include "MovieScene.h"
#include "UISequenceObjectReference.h"
#include "UISequence.generated.h"

/**
 * Movie scene animation embedded within an actor.
 */
UCLASS(BlueprintType, DefaultToInstanced)
class UISEQUENCE_API UUISequence : public UMovieSceneSequence
{
public:
	GENERATED_BODY()

	UUISequence(const FObjectInitializer& ObjectInitializer);

	//~ UMovieSceneSequence interface
	virtual void BindPossessableObject(const FGuid& ObjectId, UObject& PossessedObject, UObject* Context) override;
	virtual bool CanPossessObject(UObject& Object, UObject* InPlaybackContext) const override;
	virtual void LocateBoundObjects(const FGuid& ObjectId, UObject* Context, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const override;
#if WITH_EDITORONLY_DATA
	virtual UMovieScene* GetMovieScene() const override;
#endif
	virtual UObject* GetParentObject(UObject* Object) const override;
	virtual void UnbindPossessableObjects(const FGuid& ObjectId) override;
	virtual UObject* CreateDirectorInstance(IMovieScenePlayer& Player) override;

#if WITH_EDITOR
	virtual FText GetDisplayName() const override;
	virtual ETrackSupport IsTrackSupported(TSubclassOf<class UMovieSceneTrack> InTrackClass) const override;
#endif

	UBlueprint* GetParentBlueprint() const;

	bool IsEditable() const;

	virtual bool IsEditorOnly() const override { return true; };
	
private:
	//~ UObject interface
	virtual void PostInitProperties() override;

public:
#if WITH_EDITORONLY_DATA
	/** Pointer to the movie scene that controls this animation. */
	UPROPERTY(Instanced)
	UMovieScene* MovieScene;
#endif
	
	/** Collection of object references. */
	UPROPERTY()
	FUISequenceObjectReferenceMap ObjectReferences;

#if WITH_EDITOR
public:
	bool bChanged = false;
#endif

#if WITH_EDITOR
public:
	/** Event that is fired to initialize default state for a sequence */
	DECLARE_EVENT_OneParam(UUISequence, FOnInitialize, UUISequence*)
	static FOnInitialize& OnInitializeSequence() { return OnInitializeSequenceEvent; }

private:
	static FOnInitialize OnInitializeSequenceEvent;
#endif

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY()
	bool bHasBeenInitialized;
#endif
};