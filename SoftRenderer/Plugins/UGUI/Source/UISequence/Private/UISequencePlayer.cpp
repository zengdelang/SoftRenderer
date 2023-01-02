#include "UISequencePlayer.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SimpleConstructionScript.h"

UObject* UUISequencePlayer::GetPlaybackContext() const
{
	UUISequence* UISequence = CastChecked<UUISequence>(Sequence);
	if (UISequence)
	{
		if (AActor* Actor = UISequence->GetTypedOuter<AActor>())
		{
			return Actor;
		}
#if WITH_EDITOR
		else if (UBlueprintGeneratedClass* GeneratedClass = UISequence->GetTypedOuter<UBlueprintGeneratedClass>())
		{
			return GeneratedClass->SimpleConstructionScript->GetComponentEditorActorInstance();
		}
#endif
	}

	return nullptr;
}

TArray<UObject*> UUISequencePlayer::GetEventContexts() const
{
	TArray<UObject*> Contexts;
	if (UObject* PlaybackContext = GetPlaybackContext())
	{
		Contexts.Add(PlaybackContext);
	}
	return Contexts;
}