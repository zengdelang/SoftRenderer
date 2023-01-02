#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "Templates/SubclassOf.h"
#include "ISequencer.h"
#include "MovieSceneTrack.h"
#include "ISequencerSection.h"
#include "ISequencerTrackEditor.h"
#include "MovieSceneTrackEditor.h"

class UMaterial;
class UMaterialInterface;
class UMovieSceneUIMaterialTrack;
class USceneComponent;

/**
 * Track editor for material parameters.
 */
class UGUIEDITOR_API FUIMaterialTrackEditor
	: public FMovieSceneTrackEditor
{
public:

	/** Constructor. */
	FUIMaterialTrackEditor( TSharedRef<ISequencer> InSequencer );

	/** Virtual destructor. */
	virtual ~FUIMaterialTrackEditor() { }

public:

	// ISequencerTrackEditor interface

	virtual TSharedPtr<SWidget> BuildOutlinerEditWidget( const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params ) override;
	virtual TSharedRef<ISequencerSection> MakeSectionInterface( UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding ) override;

protected:

	/** Gets a material interface for a specific object binding and material track. */
	virtual UMaterialInterface* GetMaterialInterfaceForTrack( FGuid ObjectBinding, UMovieSceneUIMaterialTrack* MaterialTrack ) = 0;

private:

	/** Provides the contents of the add parameter menu. */
	TSharedRef<SWidget> OnGetAddParameterMenuContent( FGuid ObjectBinding, UMovieSceneUIMaterialTrack* MaterialTrack );

	/** Gets a material for a specific object binding and track */
	UMaterial* GetMaterialForTrack( FGuid ObjectBinding, UMovieSceneUIMaterialTrack* MaterialTrack );

	/** Adds a scalar parameter and initial key to a material track.
	 * @param ObjectBinding The object binding which owns the material track.
	 * @param MaterialTrack The track to Add the section to.
	 * @param ParameterName The name of the parameter to add an initial key for.
	 */
	void AddScalarParameter( FGuid ObjectBinding, UMovieSceneUIMaterialTrack* MaterialTrack, FName ParameterName );

	/** Adds a color parameter and initial key to a material track.
	* @param ObjectBinding The object binding which owns the material track.
	* @param MaterialTrack The track to Add the section to.
	* @param ParameterName The name of the parameter to add an initial key for.
	*/
	void AddColorParameter( FGuid ObjectBinding, UMovieSceneUIMaterialTrack* MaterialTrack, FName ParameterName );
};


/**
 * A specialized material track editor for component materials
 */
class FUIComponentMaterialTrackEditor
	: public FUIMaterialTrackEditor
{
public:

	FUIComponentMaterialTrackEditor( TSharedRef<ISequencer> InSequencer );

	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor( TSharedRef<ISequencer> OwningSequencer );

public:

	// ISequencerTrackEditor interface
	virtual void ExtendObjectBindingTrackMenu(TSharedRef<FExtender> Extender, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass) override;
	virtual bool SupportsType( TSubclassOf<UMovieSceneTrack> Type ) const override;
	virtual bool GetDefaultExpansionState(UMovieSceneTrack* InTrack) const override;

protected:

	// FMaterialtrackEditor interface

	virtual UMaterialInterface* GetMaterialInterfaceForTrack( FGuid ObjectBinding, UMovieSceneUIMaterialTrack* MaterialTrack ) override;

private:

	void ConstructObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, TArray<FGuid> ObjectBindings);
	/** Callback for executing the add component material track. */
	void HandleAddComponentMaterialActionExecute(USceneComponent* Component, int32 MaterialIndex);

};
