#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

class FUIComponentClassComboEntry;

DECLARE_MULTICAST_DELEGATE(FOnUIComponentTypeListChanged);

typedef TSharedPtr<class FUIComponentClassComboEntry> FUIComponentClassComboEntryPtr;

struct FUIComponentTypeEntry
{
	/** Name of the component, as typed by the user */
	FString ComponentName;

	/** Name of the component, corresponds to asset name for blueprint components */
	FString ComponentAssetName;

	/** Optional pointer to the UClass, will be nullptr for blueprint components that aren't loaded */
	class UClass* ComponentClass;
};

struct UIBLUEPRINTEDITOR_API FUIComponentTypeRegistry
{
	static FUIComponentTypeRegistry& Get();

	/**
	 * Called when the user changes the text in the search box.
	 * @OutComponentList Pointer that will be set to the (globally shared) component type list
	 * @return Deleate that can be used to handle change notifications. change notifications are raised when entries are 
	 *	added or removed from the component type list
	 */
	FOnUIComponentTypeListChanged& SubscribeToComponentList(TArray<FUIComponentClassComboEntryPtr>*& OutComponentList);
	FOnUIComponentTypeListChanged& SubscribeToComponentList(const TArray<FUIComponentTypeEntry>*& OutComponentList);
	FOnUIComponentTypeListChanged& GetOnComponentTypeListChanged();

	/**
	 * Called when a specific class has been updated and should force the component type registry to update as well
	 */
	void InvalidateClass(TSubclassOf<UActorComponent> ClassToUpdate);
private:
	void OnProjectHotReloaded( bool bWasTriggeredAutomatically );

private:
	FUIComponentTypeRegistry();
	~FUIComponentTypeRegistry();
	struct FUIComponentTypeRegistryData* Data;

public:
	static bool IsClassABlueprintSpawnableComponent(const UClass* Class);
	
};
