#include "Sequence/TrackEditors/UIMaterialTrackEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Sections/MovieSceneParameterSection.h"
#include "Sequence/Sections/UIParameterSection.h"
#include "SequencerUtilities.h"
#include "Modules/ModuleManager.h"
#include "MaterialEditorModule.h"
#include "Engine/Selection.h"
#include "ISequencerModule.h"
#include "Sequence/Tracks/MovieSceneUIMaterialTrack.h"
#include "Core/Widgets/Mesh/UIStaticMeshComponent.h"

#define LOCTEXT_NAMESPACE "UIMaterialTrackEditor"

FUIMaterialTrackEditor::FUIMaterialTrackEditor( TSharedRef<ISequencer> InSequencer )
	: FMovieSceneTrackEditor( InSequencer )
{
}

TSharedRef<ISequencerSection> FUIMaterialTrackEditor::MakeSectionInterface( UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding )
{
	UMovieSceneParameterSection* ParameterSection = Cast<UMovieSceneParameterSection>(&SectionObject);
	checkf( ParameterSection != nullptr, TEXT("Unsupported section type.") );

	return MakeShareable(new FUIParameterSection( *ParameterSection ));
}

TSharedPtr<SWidget> FUIMaterialTrackEditor::BuildOutlinerEditWidget( const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params )
{
	UMovieSceneUIMaterialTrack* MaterialTrack = Cast<UMovieSceneUIMaterialTrack>(Track);
	FOnGetContent MenuContent = FOnGetContent::CreateSP(this, &FUIMaterialTrackEditor::OnGetAddParameterMenuContent, ObjectBinding, MaterialTrack);

	return FSequencerUtilities::MakeAddButton(LOCTEXT( "AddParameterButton", "Parameter" ), MenuContent, Params.NodeIsHovered, GetSequencer());
}

struct FParameterNameAndAction
{
	FName ParameterName;
	FUIAction Action;

	FParameterNameAndAction( FName InParameterName, FUIAction InAction )
	{
		ParameterName = InParameterName;
		Action = InAction;
	}

	bool operator<(FParameterNameAndAction const& Other) const
	{
		return ParameterName.LexicalLess(Other.ParameterName);
	}
};

TSharedRef<SWidget> FUIMaterialTrackEditor::OnGetAddParameterMenuContent( FGuid ObjectBinding, UMovieSceneUIMaterialTrack* MaterialTrack )
{
	FMenuBuilder AddParameterMenuBuilder( true, nullptr );

	UMaterial* Material = GetMaterialForTrack( ObjectBinding, MaterialTrack );
	if ( Material != nullptr )
	{
		UMaterialInterface* MaterialInterface = GetMaterialInterfaceForTrack(ObjectBinding, MaterialTrack);
		
		UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>( MaterialInterface );	
		TArray<FMaterialParameterInfo> VisibleExpressions;

		IMaterialEditorModule* MaterialEditorModule = &FModuleManager::LoadModuleChecked<IMaterialEditorModule>( "MaterialEditor" );
		bool bCollectedVisibleParameters = false;
		if (MaterialEditorModule && MaterialInstance)
		{
			MaterialEditorModule->GetVisibleMaterialParameters(Material, MaterialInstance, VisibleExpressions);
			bCollectedVisibleParameters = true;
		}

		TArray<FParameterNameAndAction> ParameterNamesAndActions;

		// Collect scalar parameters.
		TArray<FMaterialParameterInfo> ScalarParameterInfo;
		TArray<FGuid> ScalarParameterGuids;
		Material->GetAllScalarParameterInfo(ScalarParameterInfo, ScalarParameterGuids );
		for (int32 ScalarParameterIndex = 0; ScalarParameterIndex < ScalarParameterInfo.Num(); ++ScalarParameterIndex)
		{
			if (!bCollectedVisibleParameters || VisibleExpressions.Contains(FMaterialParameterInfo(ScalarParameterInfo[ScalarParameterIndex].Name)))
			{
				FName ScalarParameterName = ScalarParameterInfo[ScalarParameterIndex].Name;
				FUIAction AddParameterMenuAction( FExecuteAction::CreateSP( this, &FUIMaterialTrackEditor::AddScalarParameter, ObjectBinding, MaterialTrack, ScalarParameterName ) );
				FParameterNameAndAction NameAndAction( ScalarParameterName, AddParameterMenuAction );
				ParameterNamesAndActions.Add(NameAndAction);
			}
		}

		// Collect color parameters.
		TArray<FMaterialParameterInfo> ColorParameterInfo;
		TArray<FGuid> ColorParameterGuids;
		Material->GetAllVectorParameterInfo( ColorParameterInfo, ColorParameterGuids );
		for (int32 ColorParameterIndex = 0; ColorParameterIndex < ColorParameterInfo.Num(); ++ColorParameterIndex)
		{
			if (!bCollectedVisibleParameters || VisibleExpressions.Contains(FMaterialParameterInfo(ColorParameterInfo[ColorParameterIndex].Name)))
			{
				FName ColorParameterName = ColorParameterInfo[ColorParameterIndex].Name;
				FUIAction AddParameterMenuAction( FExecuteAction::CreateSP( this, &FUIMaterialTrackEditor::AddColorParameter, ObjectBinding, MaterialTrack, ColorParameterName ) );
				FParameterNameAndAction NameAndAction( ColorParameterName, AddParameterMenuAction );
				ParameterNamesAndActions.Add( NameAndAction );
			}
		}

		// Sort and generate menu.
		ParameterNamesAndActions.Sort();
		for ( FParameterNameAndAction NameAndAction : ParameterNamesAndActions )
		{
			AddParameterMenuBuilder.AddMenuEntry( FText::FromName( NameAndAction.ParameterName ), FText(), FSlateIcon(), NameAndAction.Action );
		}
	}

	return AddParameterMenuBuilder.MakeWidget();
}

UMaterial* FUIMaterialTrackEditor::GetMaterialForTrack( FGuid ObjectBinding, UMovieSceneUIMaterialTrack* MaterialTrack )
{
	UMaterialInterface* MaterialInterface = GetMaterialInterfaceForTrack( ObjectBinding, MaterialTrack );
	if ( MaterialInterface != nullptr )
	{
		UMaterial* Material = Cast<UMaterial>( MaterialInterface );
		if ( Material != nullptr )
		{
			return Material;
		}
		else
		{
			UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>( MaterialInterface );
			if ( MaterialInstance != nullptr )
			{
				return MaterialInstance->GetMaterial();
			}
		}
	}
	return nullptr;
}

void FUIMaterialTrackEditor::AddScalarParameter( FGuid ObjectBinding, UMovieSceneUIMaterialTrack* MaterialTrack, FName ParameterName )
{
	FFrameNumber KeyTime = GetTimeForKey();

	UMaterialInterface* Material = GetMaterialInterfaceForTrack(ObjectBinding, MaterialTrack);
	if (Material != nullptr)
	{
		const FScopedTransaction Transaction( LOCTEXT( "AddScalarParameter", "Add scalar parameter" ) );
		float ParameterValue;
		Material->GetScalarParameterValue(ParameterName, ParameterValue);
		MaterialTrack->Modify();
		MaterialTrack->AddScalarParameterKey(ParameterName, KeyTime, ParameterValue);
	}
	GetSequencer()->NotifyMovieSceneDataChanged( EMovieSceneDataChangeType::MovieSceneStructureItemAdded );
}

void FUIMaterialTrackEditor::AddColorParameter( FGuid ObjectBinding, UMovieSceneUIMaterialTrack* MaterialTrack, FName ParameterName )
{
	FFrameNumber KeyTime = GetTimeForKey();

	UMaterialInterface* Material = GetMaterialInterfaceForTrack( ObjectBinding, MaterialTrack );
	if ( Material != nullptr )
	{
		const FScopedTransaction Transaction( LOCTEXT( "AddVectorParameter", "Add vector parameter" ) );
		FLinearColor ParameterValue;
		Material->GetVectorParameterValue( ParameterName, ParameterValue );
		MaterialTrack->Modify();
		MaterialTrack->AddColorParameterKey( ParameterName, KeyTime, ParameterValue );
	}
	GetSequencer()->NotifyMovieSceneDataChanged( EMovieSceneDataChangeType::MovieSceneStructureItemAdded );
}

FUIComponentMaterialTrackEditor::FUIComponentMaterialTrackEditor( TSharedRef<ISequencer> InSequencer )
	: FUIMaterialTrackEditor( InSequencer )
{
}

TSharedRef<ISequencerTrackEditor> FUIComponentMaterialTrackEditor::CreateTrackEditor( TSharedRef<ISequencer> OwningSequencer )
{
	return MakeShareable( new FUIComponentMaterialTrackEditor( OwningSequencer ) );
}

bool FUIComponentMaterialTrackEditor::SupportsType( TSubclassOf<UMovieSceneTrack> Type ) const
{
	return Type == UMovieSceneUIComponentMaterialTrack::StaticClass();
}

bool FUIComponentMaterialTrackEditor::GetDefaultExpansionState(UMovieSceneTrack* InTrack) const
{
	return true;
}

UMaterialInterface* FUIComponentMaterialTrackEditor::GetMaterialInterfaceForTrack( FGuid ObjectBinding, UMovieSceneUIMaterialTrack* MaterialTrack )
{
	TSharedPtr<ISequencer> SequencerPtr = GetSequencer();
	if (!SequencerPtr.IsValid())
	{
		return nullptr;
	}

	UMovieSceneUIComponentMaterialTrack* ComponentMaterialTrack = Cast<UMovieSceneUIComponentMaterialTrack>( MaterialTrack );
	if (!ComponentMaterialTrack)
	{
		return nullptr;
	}

	UObject* Object = GetSequencer()->FindSpawnedObjectOrTemplate(ObjectBinding);
	if (!Object)
	{
		return nullptr;
	}

	if (IUIPrimitiveElementInterface* Component = Cast<IUIPrimitiveElementInterface>(Object))
	{
		return Component->GetOverrideMaterial( ComponentMaterialTrack->GetMaterialIndex() );
	}

	return nullptr;
}

void FUIComponentMaterialTrackEditor::ExtendObjectBindingTrackMenu(TSharedRef<FExtender> Extender, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass)
{
	if (ObjectClass->ImplementsInterface(UUIPrimitiveElementInterface::StaticClass()))
	{
		Extender->AddMenuExtension(SequencerMenuExtensionPoints::AddTrackMenu_PropertiesSection, EExtensionHook::Before, nullptr, FMenuExtensionDelegate::CreateSP(this, &FUIComponentMaterialTrackEditor::ConstructObjectBindingTrackMenu, ObjectBindings));
	}
}

void FUIComponentMaterialTrackEditor::ConstructObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, TArray<FGuid> ObjectBindings)
{
	UObject* Object = GetSequencer()->FindSpawnedObjectOrTemplate(ObjectBindings[0]);
	if (!Object)
	{
		return;
	}

	USceneComponent* SceneComponent = Cast<USceneComponent>(Object);
	if (!SceneComponent)
	{
		return;
	}

	if (IUIPrimitiveElementInterface* Component = Cast<IUIPrimitiveElementInterface>(SceneComponent))
	{
		const int32 NumMaterials = Component->GetNumOverrideMaterials();
		if (NumMaterials > 0)
		{
			MenuBuilder.BeginSection("Materials", LOCTEXT("MaterialSection", "Material Parameters"));
			{
				for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; MaterialIndex++)
				{
					FUIAction AddComponentMaterialAction(FExecuteAction::CreateRaw(this, &FUIComponentMaterialTrackEditor::HandleAddComponentMaterialActionExecute, SceneComponent, MaterialIndex));
					FText AddComponentMaterialLabel = FText::Format(LOCTEXT("ComponentMaterialIndexLabelFormat", "Element {0}"), FText::AsNumber(MaterialIndex));
					FText AddComponentMaterialToolTip = FText::Format(LOCTEXT("ComponentMaterialIndexToolTipFormat", "Add material element {0}"), FText::AsNumber(MaterialIndex));
					MenuBuilder.AddMenuEntry(AddComponentMaterialLabel, AddComponentMaterialToolTip, FSlateIcon(), AddComponentMaterialAction);
				}
			}
			MenuBuilder.EndSection();
		}
	}
}

void FUIComponentMaterialTrackEditor::HandleAddComponentMaterialActionExecute(USceneComponent* Component, int32 MaterialIndex)
{
	TSharedPtr<ISequencer> SequencerPtr = GetSequencer();
	UMovieScene* MovieScene = SequencerPtr->GetFocusedMovieSceneSequence()->GetMovieScene();
	if (MovieScene->IsReadOnly())
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddComponentMaterialTrack", "Add component material track"));

	MovieScene->Modify();

	FString ComponentName = Component->GetName();

	TArray<UActorComponent*> ActorComponents;
	ActorComponents.Add(Component);

	USelection* SelectedActors = GEditor->GetSelectedActors();
	if (SelectedActors && SelectedActors->Num() > 0)
	{
		for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
		{
			AActor* Actor = CastChecked<AActor>(*Iter);

			TArray<UActorComponent*> OutActorComponents;
			Actor->GetComponents(OutActorComponents);
			for (UActorComponent* ActorComponent : OutActorComponents)
			{
				if (ActorComponent->GetName() == ComponentName)
				{
					ActorComponents.AddUnique(ActorComponent);
				}
			}
		}
	}

	for (UActorComponent* ActorComponent : ActorComponents)
	{
		FGuid ObjectHandle = SequencerPtr->GetHandleToObject(ActorComponent);
		FName IndexName(*FString::FromInt(MaterialIndex));
		if (MovieScene->FindTrack(UMovieSceneUIComponentMaterialTrack::StaticClass(), ObjectHandle, IndexName) == nullptr)
		{
			UMovieSceneUIComponentMaterialTrack* MaterialTrack = MovieScene->AddTrack<UMovieSceneUIComponentMaterialTrack>(ObjectHandle);
			MaterialTrack->Modify();
			MaterialTrack->SetMaterialIndex(MaterialIndex);
		}
	}

	SequencerPtr->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
}

#undef LOCTEXT_NAMESPACE
