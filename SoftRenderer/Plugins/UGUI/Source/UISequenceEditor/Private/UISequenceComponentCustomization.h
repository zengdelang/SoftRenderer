#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Widgets/Layout/SBox.h"
#include "Input/Reply.h"
#include "UObject/WeakObjectPtr.h"
#include "Framework/Docking/TabManager.h"

class IDetailsView;
class UUISequence;
class UUISequenceComponent;
class ISequencer;
class FSCSEditorTreeNode;
class IPropertyUtilities;

class FUISequenceComponentCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	FReply InvokeSequencer();
	UUISequence* GetUISequence() const;

	TWeakObjectPtr<UUISequenceComponent> WeakSequenceComponent;
	TWeakPtr<FTabManager> WeakTabManager;
	TSharedPtr<SBox> InlineSequencer;
	TSharedPtr<IPropertyUtilities> PropertyUtilities;
	
};