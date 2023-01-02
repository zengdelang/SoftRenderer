#pragma once

#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"

class FBlueprintEditor;
class SUISequenceEditorWidgetImpl;
class UUISequence;
class UUISequenceComponent;

class SUISequenceEditorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUISequenceEditorWidget){}
	SLATE_END_ARGS();

	void Construct(const FArguments&, TWeakPtr<FBlueprintEditor> InBlueprintEditor);
	void AssignSequence(UUISequence* NewUISequence);
	
	UUISequence* GetSequence() const;
	FText GetDisplayLabel() const;

private:
	TWeakPtr<SUISequenceEditorWidgetImpl> Impl;
	
};

struct FUISequenceEditorSummoner : public FWorkflowTabFactory
{
public:
	FUISequenceEditorSummoner(TSharedPtr<FBlueprintEditor> BlueprintEditor);

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;

protected:
	TWeakObjectPtr<UUISequenceComponent> WeakComponent;
	TWeakPtr<FBlueprintEditor> WeakBlueprintEditor;
	
};
