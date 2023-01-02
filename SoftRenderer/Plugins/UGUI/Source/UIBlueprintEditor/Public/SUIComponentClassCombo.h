#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Templates/SubclassOf.h"
#include "Components/ActorComponent.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"

class FUIComponentClassComboEntry;
class SToolTip;
class FTextFilterExpressionEvaluator;

typedef TSharedPtr<class FUIComponentClassComboEntry> FUIComponentClassComboEntryPtr;

//////////////////////////////////////////////////////////////////////////

namespace EUIComponentCreateAction
{
	enum Type
	{
		/** Create a new C++ class based off the specified ActorComponent class and then add it to the tree */
		CreateNewCPPClass,
		/** Create a new blueprint class based off the specified ActorComponent class and then add it to the tree */
		CreateNewBlueprintClass,
		/** Spawn a new instance of the specified ActorComponent class and then add it to the tree */
		SpawnExistingClass,
	};
}


DECLARE_DELEGATE_OneParam(FOnUIComponentCreated, UActorComponent*);

DECLARE_DELEGATE_RetVal_ThreeParams( UActorComponent*, FUIComponentClassSelected, TSubclassOf<UActorComponent>, EUIComponentCreateAction::Type, UObject*);

struct FUIComponentEntryCustomizationArgs
{
	/** Specific asset to use instead of the selected asset in the content browser */
	TWeakObjectPtr<UObject> AssetOverride;
	/** Custom name to display */
	FString ComponentNameOverride;
	/** Callback when a new component is created */
	FOnUIComponentCreated OnComponentCreated;
	/** Brush icon to use instead of the class icon */
	FName IconOverrideBrushName;
	/** Custom sort priority to use (smaller means sorted first) */
	int32 SortPriority;

	FUIComponentEntryCustomizationArgs()
		: AssetOverride( nullptr )
		, ComponentNameOverride()
		, OnComponentCreated()
		, IconOverrideBrushName( NAME_None )
		, SortPriority(0)
	{
	
	}
};
class FUIComponentClassComboEntry: public TSharedFromThis<FUIComponentClassComboEntry>
{
public:
	FUIComponentClassComboEntry( const FString& InHeadingText, TSubclassOf<UActorComponent> InComponentClass, bool InIncludedInFilter, EUIComponentCreateAction::Type InComponentCreateAction, FUIComponentEntryCustomizationArgs InCustomizationArgs = FUIComponentEntryCustomizationArgs() )
		: ComponentClass(InComponentClass)
		, IconClass(InComponentClass)
		, ComponentName()
		, ComponentPath()
		, HeadingText(InHeadingText)
		, bIncludedInFilter(InIncludedInFilter)
		, ComponentCreateAction(InComponentCreateAction)
		, CustomizationArgs(InCustomizationArgs)
	{}

	FUIComponentClassComboEntry(const FString& InHeadingText, const FString& InComponentName, FName InComponentPath, const UClass* InIconClass, bool InIncludedInFilter)
		: ComponentClass(nullptr)
		, IconClass(InIconClass)
		, ComponentName(InComponentName)
		, ComponentPath(InComponentPath)
		, HeadingText(InHeadingText)
		, bIncludedInFilter(InIncludedInFilter)
		, ComponentCreateAction(EUIComponentCreateAction::SpawnExistingClass)
	{}

	FUIComponentClassComboEntry( const FString& InHeadingText )
		: ComponentClass(nullptr)
		, IconClass(nullptr)
		, ComponentName()
		, ComponentPath()
		, HeadingText(InHeadingText)
		, bIncludedInFilter(false)
	{}

	FUIComponentClassComboEntry()
		: ComponentClass(nullptr)
		, IconClass(nullptr)
	{}


	TSubclassOf<UActorComponent> GetComponentClass() const { return ComponentClass; }

	const UClass* GetIconClass() const { return IconClass; }

	const FString& GetHeadingText() const { return HeadingText; }

	bool IsHeading() const
	{
		return ((ComponentClass == NULL && ComponentName == FString()) && !HeadingText.IsEmpty());
	}
	bool IsSeparator() const
	{
		return ((ComponentClass == NULL && ComponentName == FString()) && HeadingText.IsEmpty());
	}
	
	bool IsClass() const
	{
		return (ComponentClass != NULL || ComponentName != FString());
	}
	
	bool IsIncludedInFilter() const
	{
		return bIncludedInFilter;
	}
	
	const FString& GetComponentNameOverride() const
	{
		return CustomizationArgs.ComponentNameOverride;
	}

	EUIComponentCreateAction::Type GetComponentCreateAction() const
	{
		return ComponentCreateAction;
	}

	FOnUIComponentCreated& GetOnComponentCreated()
	{
		return CustomizationArgs.OnComponentCreated;
	}
	FString GetClassName() const;
	FString GetComponentPath() const { return ComponentPath.ToString(); }

	UObject* GetAssetOverride()
	{
		return CustomizationArgs.AssetOverride.Get();
	}

	FName GetIconOverrideBrushName() const { return CustomizationArgs.IconOverrideBrushName; }

	int32 GetSortPriority() const { return CustomizationArgs.SortPriority; }

	void AddReferencedObjects(FReferenceCollector& Collector);
private:
	TSubclassOf<UActorComponent> ComponentClass;
	const UClass* IconClass;
	// For components that are not loaded we just keep the name of the component,
	// loading occurs when the blueprint is spawned, which should also trigger a refresh
	// of the component list:
	FString ComponentName;
	FName ComponentPath;
	FString HeadingText;
	bool bIncludedInFilter;
	EUIComponentCreateAction::Type ComponentCreateAction;
	FUIComponentEntryCustomizationArgs CustomizationArgs;
};

//////////////////////////////////////////////////////////////////////////

class UIBLUEPRINTEDITOR_API SUIComponentClassCombo : public SComboButton
{
public:
	SLATE_BEGIN_ARGS( SUIComponentClassCombo )
		: _IncludeText(true)
	{}

		SLATE_ATTRIBUTE(bool, IncludeText)
		SLATE_EVENT( FUIComponentClassSelected, OnComponentClassSelected )

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual ~SUIComponentClassCombo() override;

	/** Clear the current combo list selection */
	void ClearSelection();

	/**
	 * Updates the filtered list of component names.
	 */
	void GenerateFilteredComponentList();

	FText GetCurrentSearchString() const;

	/**
	 * Called when the user changes the text in the search box.
	 * @param InSearchText The new search string.
	 */
	void OnSearchBoxTextChanged( const FText& InSearchText );

	/** Callback when the user commits the text in the searchbox */
	void OnSearchBoxTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo);

	void OnAddComponentSelectionChanged( FUIComponentClassComboEntryPtr InItem, ESelectInfo::Type SelectInfo );

	TSharedRef<ITableRow> GenerateAddComponentRow( FUIComponentClassComboEntryPtr Entry, const TSharedRef<STableViewBase> &OwnerTable ) const;

	/** Update list of component classes */
	void UpdateComponentClassList();

	/** Returns a component name without the substring "Component" and sanitized for display */
	static FString GetSanitizedComponentName(FUIComponentClassComboEntryPtr Entry);

private:

	FText GetFriendlyComponentName(FUIComponentClassComboEntryPtr Entry) const;

	TSharedRef<SToolTip> GetComponentToolTip(FUIComponentClassComboEntryPtr Entry) const;

	FUIComponentClassSelected OnComponentClassSelected;

	/** List of component class names used by combo box */
	TArray<FUIComponentClassComboEntryPtr>* ComponentClassList;

	/** List of component class names, filtered by the current search string */
	TArray<FUIComponentClassComboEntryPtr> FilteredComponentClassList;

	/** The current search string */
	TSharedPtr<FTextFilterExpressionEvaluator> TextFilter;

	/** The search box control - part of the combo drop down */
	TSharedPtr<SSearchBox> SearchBox;

	/** The component list control - part of the combo drop down */
	TSharedPtr< SListView<FUIComponentClassComboEntryPtr> > ComponentClassListView;

	/** Cached selection index used to skip over unselectable items */
	int32 PrevSelectedIndex;
};
