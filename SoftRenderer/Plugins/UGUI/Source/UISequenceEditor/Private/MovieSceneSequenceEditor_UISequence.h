#pragma once

#include "MovieSceneSequenceEditor.h"
#include "UISequence.h"
#include "Engine/Level.h"
#include "Engine/LevelScriptBlueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Tracks/MovieSceneEventTrack.h"
#include "K2Node_FunctionEntry.h"
#include "EdGraphSchema_K2.h"

struct FMovieSceneSequenceEditor_UISequence : FMovieSceneSequenceEditor
{
	virtual bool CanCreateEvents(UMovieSceneSequence* InSequence) const
	{
		return true;
	}

	virtual UBlueprint* GetBlueprintForSequence(UMovieSceneSequence* InSequence) const override
	{
		UUISequence* UISequence = CastChecked<UUISequence>(InSequence);
		if (UBlueprint* Blueprint = UISequence->GetParentBlueprint())
		{
			return Blueprint;
		}

		UUISequenceComponent* Component = UISequence->GetTypedOuter<UUISequenceComponent>();
		ULevel* Level = Component ? Component->GetOwner()->GetLevel() : nullptr;

		bool bDontCreateNewBlueprint = true;
		return Level ? Level->GetLevelScriptBlueprint(bDontCreateNewBlueprint) : nullptr;
	}

	virtual UBlueprint* CreateBlueprintForSequence(UMovieSceneSequence* InSequence) const override
	{
		UUISequence* UISequence = CastChecked<UUISequence>(InSequence);
		check(!UISequence->GetParentBlueprint());

		UUISequenceComponent* Component = UISequence->GetTypedOuter<UUISequenceComponent>();
		ULevel* Level = Component ? Component->GetOwner()->GetLevel() : nullptr;

		bool bDontCreateNewBlueprint = false;
		return Level ? Level->GetLevelScriptBlueprint(bDontCreateNewBlueprint) : nullptr;
	}
};