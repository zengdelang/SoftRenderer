#pragma once

class UISEQUENCEEDITOR_API FUISequenceEditorSelection
{
public:
	static TMap<TWeakObjectPtr<AActor>, TSet<TWeakObjectPtr<UActorComponent>>> SequenceActorSelection; 
};
