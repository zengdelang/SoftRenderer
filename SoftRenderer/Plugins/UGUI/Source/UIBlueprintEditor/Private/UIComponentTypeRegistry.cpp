#include "UIComponentTypeRegistry.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectIterator.h"
#include "Misc/PackageName.h"
#include "Engine/Blueprint.h"
#include "Components/StaticMeshComponent.h"
#include "TickableEditorObject.h"
#include "ActorFactories/ActorFactoryBasicShape.h"
#include "Materials/Material.h"
#include "Engine/StaticMesh.h"
#include "AssetRegistryModule.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/HotReloadInterface.h"
#include "SUIComponentClassCombo.h"
#include "Settings/ClassViewerSettings.h"

#define LOCTEXT_NAMESPACE "ComponentTypeRegistry"

//////////////////////////////////////////////////////////////////////////
// FUIComponentTypeRegistryData

struct FUIComponentTypeRegistryData
	: public FTickableEditorObject
	, public FGCObject
{
	FUIComponentTypeRegistryData();

	// Force a refresh of the components list right now (also calls the ComponentListChanged delegate to notify watchers)
	void ForceRefreshComponentList();

	static void AddBasicShapeComponents(TArray<FUIComponentClassComboEntryPtr>& SortedClassList);

	/** Implementation of FTickableEditorObject */
	virtual void Tick(float) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(FTypeDatabaseUpdater, STATGROUP_Tickables); }
	
	/** Implementation of FGCObject */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override
	{
		return "FUIComponentTypeRegistryData";
	}
	
	// Request a refresh of the components list next frame
	void Invalidate()
	{
		bNeedsRefreshNextTick = true;
	}
public:
	/** End implementation of FTickableEditorObject */
	TArray<FUIComponentClassComboEntryPtr> ComponentClassList;
	TArray<FUIComponentTypeEntry> ComponentTypeList;
	TArray<FAssetData> PendingAssetData;
	FOnUIComponentTypeListChanged ComponentListChanged;
	bool bNeedsRefreshNextTick;
};

static const FString CommonClassGroup(TEXT("Common"));
// This has to stay in sync with logic in FKismetCompilerContext::FinishCompilingClass
static const FString BlueprintComponents(TEXT("Custom"));

template <typename ObjectType>
static ObjectType* FindOrLoadObject( const FString& ObjectPath )
{
	ObjectType* Object = FindObject<ObjectType>( nullptr, *ObjectPath );
	if( !Object )
	{
		Object = LoadObject<ObjectType>( nullptr, *ObjectPath );
	}

	return Object;
}

void FUIComponentTypeRegistryData::AddBasicShapeComponents(TArray<FUIComponentClassComboEntryPtr>& SortedClassList)
{
	FString BasicShapesHeading = LOCTEXT("BasicShapesHeading", "Basic Shapes").ToString();

	const auto OnBasicShapeCreated = [](UActorComponent* Component)
	{
		UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(Component);
		if (SMC)
		{
			const FString MaterialName = TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial");
			UMaterial* MaterialAsset = FindOrLoadObject<UMaterial>(MaterialName);
			SMC->SetMaterial(0, MaterialAsset);

			// If the component object is an archetype (template), propagate the material setting to any instances, as instances
			// of the archetype will end up being created BEFORE we are able to set the override material on the template object.
			if (SMC->HasAnyFlags(RF_ArchetypeObject))
			{
				TArray<UObject*> ArchetypeInstances;
				SMC->GetArchetypeInstances(ArchetypeInstances);
				for (UObject* ArchetypeInstance : ArchetypeInstances)
				{
					CastChecked<UStaticMeshComponent>(ArchetypeInstance)->SetMaterial(0, MaterialAsset);
				}
			}
		}
	};

	{
		FUIComponentEntryCustomizationArgs Args;
		Args.AssetOverride = FindOrLoadObject<UStaticMesh>(UActorFactoryBasicShape::BasicCube.ToString());
		Args.OnComponentCreated = FOnUIComponentCreated::CreateStatic(OnBasicShapeCreated);
		Args.ComponentNameOverride = LOCTEXT("BasicCubeShapeDisplayName", "Cube").ToString();
		Args.IconOverrideBrushName = FName("ClassIcon.Cube");
		Args.SortPriority = 2;

		{
			FUIComponentClassComboEntryPtr NewShape = MakeShareable(new FUIComponentClassComboEntry(BasicShapesHeading, UStaticMeshComponent::StaticClass(), true, EUIComponentCreateAction::SpawnExistingClass, Args));
			SortedClassList.Add(NewShape);
		}

		{
			//Cube also goes in the common group
			FUIComponentClassComboEntryPtr NewShape = MakeShareable(new FUIComponentClassComboEntry(CommonClassGroup, UStaticMeshComponent::StaticClass(), false, EUIComponentCreateAction::SpawnExistingClass, Args));
			SortedClassList.Add(NewShape);
		}
	}

	{
		FUIComponentEntryCustomizationArgs Args;
		Args.AssetOverride = FindOrLoadObject<UStaticMesh>(UActorFactoryBasicShape::BasicPlane.ToString());
		Args.OnComponentCreated = FOnUIComponentCreated::CreateStatic(OnBasicShapeCreated);
		Args.ComponentNameOverride = LOCTEXT("BasicPlaneShapeDisplayName", "Plane").ToString();
		Args.IconOverrideBrushName = FName("ClassIcon.Plane");
		Args.SortPriority = 2;

		{
			FUIComponentClassComboEntryPtr NewShape = MakeShareable(new FUIComponentClassComboEntry(BasicShapesHeading, UStaticMeshComponent::StaticClass(), true, EUIComponentCreateAction::SpawnExistingClass, Args));
			SortedClassList.Add(NewShape);
		}

		{
			//Quad also goes in the common group
			FUIComponentClassComboEntryPtr NewShape = MakeShareable(new FUIComponentClassComboEntry(CommonClassGroup, UStaticMeshComponent::StaticClass(), false, EUIComponentCreateAction::SpawnExistingClass, Args));
			SortedClassList.Add(NewShape);
		}
	}

	{
		FUIComponentEntryCustomizationArgs Args;
		Args.AssetOverride = FindOrLoadObject<UStaticMesh>(UActorFactoryBasicShape::BasicSphere.ToString());
		Args.OnComponentCreated = FOnUIComponentCreated::CreateStatic(OnBasicShapeCreated);
		Args.ComponentNameOverride = LOCTEXT("BasicSphereShapeDisplayName", "Sphere").ToString();
		Args.IconOverrideBrushName = FName("ClassIcon.Sphere");
		Args.SortPriority = 2;
		{
			FUIComponentClassComboEntryPtr NewShape = MakeShareable(new FUIComponentClassComboEntry(BasicShapesHeading, UStaticMeshComponent::StaticClass(), true, EUIComponentCreateAction::SpawnExistingClass, Args));
			SortedClassList.Add(NewShape);
		}

		{
			// Sphere also goes in the common group
			FUIComponentClassComboEntryPtr NewShape = MakeShareable(new FUIComponentClassComboEntry(CommonClassGroup, UStaticMeshComponent::StaticClass(), false, EUIComponentCreateAction::SpawnExistingClass, Args));
			SortedClassList.Add(NewShape);
		}
	}

	{
		FUIComponentEntryCustomizationArgs Args;
		Args.AssetOverride = FindOrLoadObject<UStaticMesh>(UActorFactoryBasicShape::BasicCylinder.ToString());
		Args.OnComponentCreated = FOnUIComponentCreated::CreateStatic(OnBasicShapeCreated);
		Args.ComponentNameOverride = LOCTEXT("BasicCylinderShapeDisplayName", "Cylinder").ToString();
		Args.IconOverrideBrushName = FName("ClassIcon.Cylinder");
		Args.SortPriority = 3;
		FUIComponentClassComboEntryPtr NewShape = MakeShareable(new FUIComponentClassComboEntry(BasicShapesHeading, UStaticMeshComponent::StaticClass(), true, EUIComponentCreateAction::SpawnExistingClass, Args));
		SortedClassList.Add(NewShape);
	}

	{
		FUIComponentEntryCustomizationArgs Args;
		Args.AssetOverride = FindOrLoadObject<UStaticMesh>(UActorFactoryBasicShape::BasicCone.ToString());
		Args.OnComponentCreated = FOnUIComponentCreated::CreateStatic(OnBasicShapeCreated);
		Args.ComponentNameOverride = LOCTEXT("BasicConeShapeDisplayName", "Cone").ToString();
		Args.IconOverrideBrushName = FName("ClassIcon.Cone");
		Args.SortPriority = 4;
		FUIComponentClassComboEntryPtr NewShape = MakeShareable(new FUIComponentClassComboEntry(BasicShapesHeading, UStaticMeshComponent::StaticClass(), true, EUIComponentCreateAction::SpawnExistingClass, Args));
		SortedClassList.Add(NewShape);
	}
}

FUIComponentTypeRegistryData::FUIComponentTypeRegistryData()
	: bNeedsRefreshNextTick(false)
{
	const auto HandleAdded = [](const FAssetData& Data, FUIComponentTypeRegistryData* Parent)
	{
		Parent->PendingAssetData.Push(Data);
	};

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().OnAssetAdded().AddStatic(HandleAdded, this);
	AssetRegistryModule.Get().OnAssetRemoved().AddStatic(HandleAdded, this);

	const auto HandleRenamed = [](const FAssetData& Data, const FString&, FUIComponentTypeRegistryData* Parent)
	{
		Parent->PendingAssetData.Push(Data);
	};
	AssetRegistryModule.Get().OnAssetRenamed().AddStatic(HandleRenamed, this);
}

void FUIComponentTypeRegistryData::ForceRefreshComponentList()
{
	bNeedsRefreshNextTick = false;
	ComponentClassList.Empty();
	ComponentTypeList.Empty();

	struct SortComboEntry
	{
		bool operator () (const FUIComponentClassComboEntryPtr& A, const FUIComponentClassComboEntryPtr& B) const
		{
			bool bResult = false;

			// check headings first, if they are the same compare the individual entries
			int32 HeadingCompareResult = FCString::Stricmp(*A->GetHeadingText(), *B->GetHeadingText());
			if (HeadingCompareResult == 0)
			{
				if( A->GetSortPriority() == 0 && B->GetSortPriority() == 0 )
				{
					bResult = FCString::Stricmp(*A->GetClassName(), *B->GetClassName()) < 0;
				}
				else
				{
					bResult = A->GetSortPriority() < B->GetSortPriority();
				}
			}
			else if (CommonClassGroup == A->GetHeadingText())
			{
				bResult = true;
			}
			else if (CommonClassGroup == B->GetHeadingText())
			{
				bResult = false;
			}
			else
			{
				bResult = HeadingCompareResult < 0;
			}

			return bResult;
		}
	};

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	/*{
		FString NewComponentsHeading = LOCTEXT("NewComponentsHeading", "Scripting").ToString();
		// Add new C++ component class
		FUIComponentClassComboEntryPtr NewClassHeader = MakeShareable(new FUIComponentClassComboEntry(NewComponentsHeading));
		ComponentClassList.Add(NewClassHeader);

		FUIComponentClassComboEntryPtr NewBPClass = MakeShareable(new FUIComponentClassComboEntry(NewComponentsHeading, UActorComponent::StaticClass(), true, EUIComponentCreateAction::CreateNewBlueprintClass));
		ComponentClassList.Add(NewBPClass);

		FUIComponentClassComboEntryPtr NewCPPClass = MakeShareable(new FUIComponentClassComboEntry(NewComponentsHeading, UActorComponent::StaticClass(), true, EUIComponentCreateAction::CreateNewCPPClass));
		ComponentClassList.Add(NewCPPClass);

		FUIComponentClassComboEntryPtr NewSeparator(new FUIComponentClassComboEntry());
		ComponentClassList.Add(NewSeparator);
	}*/

	TArray<FUIComponentClassComboEntryPtr> SortedClassList;

	//AddBasicShapeComponents(SortedClassList);

	TArray<FName> InMemoryClasses;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		// If this is a subclass of Actor Component, not abstract, and tagged as spawnable from Kismet
		if (Class->IsChildOf(UActorComponent::StaticClass()))
		{
			InMemoryClasses.Push(Class->GetFName());

			const bool bOutOfDateClass = Class->HasAnyClassFlags(CLASS_NewerVersionExists);
			const bool bBlueprintSkeletonClass = FKismetEditorUtilities::IsClassABlueprintSkeleton(Class);
			const bool bPassesAllowedClasses = GetDefault<UClassViewerSettings>()->AllowedClasses.Num() == 0 || GetDefault<UClassViewerSettings>()->AllowedClasses.Contains(Class->GetName());

			if (!bOutOfDateClass &&
				!bBlueprintSkeletonClass &&
				bPassesAllowedClasses)
			{
				if (FUIComponentTypeRegistry::IsClassABlueprintSpawnableComponent(Class))
				{
					TArray<FString> ClassGroupNames;
					Class->GetClassGroupNames(ClassGroupNames);

					if (ClassGroupNames.Contains(CommonClassGroup))
					{
						FString ClassGroup = CommonClassGroup;
						FUIComponentClassComboEntryPtr NewEntry(new FUIComponentClassComboEntry(ClassGroup, Class, ClassGroupNames.Num() <= 1, EUIComponentCreateAction::SpawnExistingClass));
						SortedClassList.Add(NewEntry);
					}
					if (ClassGroupNames.Num() && !ClassGroupNames[0].Equals(CommonClassGroup))
					{
						const bool bIncludeInFilter = true;

						FString ClassGroup = ClassGroupNames[0];
						FUIComponentClassComboEntryPtr NewEntry(new FUIComponentClassComboEntry(ClassGroup, Class, bIncludeInFilter, EUIComponentCreateAction::SpawnExistingClass));
						SortedClassList.Add(NewEntry);
					}
					else if (ClassGroupNames.Num() == 0)
					{
						// No class group name found. Just add it to a "custom" category

						const bool bIncludeInFilter = true;
						FString ClassGroup = LOCTEXT("CustomClassGroup", "Custom").ToString();
						FUIComponentClassComboEntryPtr NewEntry(new FUIComponentClassComboEntry(ClassGroup, Class, bIncludeInFilter, EUIComponentCreateAction::SpawnExistingClass));
						SortedClassList.Add(NewEntry);
					}
				}

				FUIComponentTypeEntry Entry = { Class->GetName(), FString(), Class };
				ComponentTypeList.Add(Entry);
			}
		}
	}

	/*{
		// make sure that we add any user created classes immediately, generally this will not create anything (because assets have not been discovered yet), 
		// but asset discovery should be allowed to take place at any time:
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		TArray<FName> ClassNames;
		ClassNames.Add(UActorComponent::StaticClass()->GetFName());
		TSet<FName> DerivedClassNames;
		AssetRegistryModule.Get().GetDerivedClassNames(ClassNames, TSet<FName>(), DerivedClassNames);

		TSet<FName> InMemoryClassesSet = TSet<FName>(InMemoryClasses);
		TSet<FName> OnDiskClasses = DerivedClassNames.Difference(InMemoryClassesSet);

		if (OnDiskClasses.Num() > 0)
		{
			// GetAssetsByClass call is a kludge to get the full asset paths for the blueprints we care about, Bob T. thinks 
			// that the Asset Registry could just keep asset paths:
			TArray<FAssetData> BlueprintAssetData;
			AssetRegistryModule.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetFName(), BlueprintAssetData, true);
			TMap<FString, FAssetData> BlueprintNames;
			for (const FAssetData& Blueprint : BlueprintAssetData)
			{
				BlueprintNames.Add(Blueprint.AssetName.ToString(), Blueprint);
			}

			for (const FName& OnDiskClass : OnDiskClasses)
			{
				FString FixedString = OnDiskClass.ToString();
				FixedString.RemoveFromEnd(TEXT("_C"));

				const bool bPassesAllowedClasses = GetDefault<UClassViewerSettings>()->AllowedClasses.Num() == 0 || GetDefault<UClassViewerSettings>()->AllowedClasses.Contains(FixedString);
				if (bPassesAllowedClasses)
				{
					FAssetData AssetData;
					if (const FAssetData* Value = BlueprintNames.Find(FixedString))
					{
						AssetData = *Value;
					}

					FUIComponentTypeEntry Entry = { FixedString, AssetData.ObjectPath.ToString(), nullptr };
					ComponentTypeList.Add(Entry);

					// The blueprint is unloaded, so we need to work out which icon to use for it using its asset data
					const UClass* BlueprintIconClass = FClassIconFinder::GetIconClassForAssetData(AssetData);

					const bool bIncludeInFilter = true;
					FUIComponentClassComboEntryPtr NewEntry(new FUIComponentClassComboEntry(BlueprintComponents, FixedString, AssetData.ObjectPath, BlueprintIconClass, bIncludeInFilter));
					SortedClassList.Add(NewEntry);
				}
			}
		}
	}*/

	if (SortedClassList.Num() > 0)
	{
		Sort(SortedClassList.GetData(), SortedClassList.Num(), SortComboEntry());

		FString PreviousHeading;
		for (int32 ClassIndex = 0; ClassIndex < SortedClassList.Num(); ClassIndex++)
		{
			FUIComponentClassComboEntryPtr& CurrentEntry = SortedClassList[ClassIndex];

			const FString& CurrentHeadingText = CurrentEntry->GetHeadingText();
			if (CurrentHeadingText != PreviousHeading)
			{
				// This avoids a redundant separator being added to the very top of the list
				if (ClassIndex > 0)
				{
					FUIComponentClassComboEntryPtr NewSeparator(new FUIComponentClassComboEntry());
					ComponentClassList.Add(NewSeparator);
				}
				FUIComponentClassComboEntryPtr NewHeading(new FUIComponentClassComboEntry(CurrentHeadingText));
				ComponentClassList.Add(NewHeading);

				PreviousHeading = CurrentHeadingText;
			}

			ComponentClassList.Add(CurrentEntry);
		}
	}

	ComponentListChanged.Broadcast();
}

void FUIComponentTypeRegistryData::Tick(float)
{
	bool bRequiresRefresh = bNeedsRefreshNextTick;

	if (PendingAssetData.Num() != 0)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		// Avoid querying the asset registry until it has finished discovery, 
		// as doing so may force it to update temporary caches many times:
		if(AssetRegistryModule.Get().IsLoadingAssets())
		{
			return;
		}

		TArray<FName> ClassNames;
		ClassNames.Add(UActorComponent::StaticClass()->GetFName());
		TSet<FName> DerivedClassNames;
		AssetRegistryModule.Get().GetDerivedClassNames(ClassNames, TSet<FName>(), DerivedClassNames);

		for (const FAssetData& Asset : PendingAssetData)
		{
			const FName BPParentClassName(GET_MEMBER_NAME_CHECKED(UBlueprint, ParentClass));
			const FString TagValue = Asset.GetTagValueRef<FString>(BPParentClassName);
			const FString ObjectPath = FPackageName::ExportTextPathToObjectPath(TagValue);
			FName ObjectName = FName(*FPackageName::ObjectPathToObjectName(ObjectPath));
			if (DerivedClassNames.Contains(ObjectName))
			{
				bRequiresRefresh = true;
				break;
			}
		}
		PendingAssetData.Empty();
	}

	if (bRequiresRefresh)
	{
		ForceRefreshComponentList();
	}
}

void FUIComponentTypeRegistryData::AddReferencedObjects(FReferenceCollector& Collector)
{
	for(FUIComponentClassComboEntryPtr& ComboEntry : ComponentClassList)
	{
		ComboEntry->AddReferencedObjects(Collector);
	}

	for(FUIComponentTypeEntry& TypeEntry : ComponentTypeList)
	{
		Collector.AddReferencedObject(TypeEntry.ComponentClass);
	}
}

//////////////////////////////////////////////////////////////////////////
// FUIComponentTypeRegistry

FUIComponentTypeRegistry& FUIComponentTypeRegistry::Get()
{
	static FUIComponentTypeRegistry ComponentRegistry;
	return ComponentRegistry;
}

FOnUIComponentTypeListChanged& FUIComponentTypeRegistry::SubscribeToComponentList(TArray<FUIComponentClassComboEntryPtr>*& OutComponentList)
{
	check(Data);
	OutComponentList = &Data->ComponentClassList;
	return Data->ComponentListChanged;
}

FOnUIComponentTypeListChanged& FUIComponentTypeRegistry::SubscribeToComponentList(const TArray<FUIComponentTypeEntry>*& OutComponentList)
{
	check(Data);
	OutComponentList = &Data->ComponentTypeList;
	return Data->ComponentListChanged;
}

FOnUIComponentTypeListChanged& FUIComponentTypeRegistry::GetOnComponentTypeListChanged()
{
	check(Data);
	return Data->ComponentListChanged;
}

FUIComponentTypeRegistry::FUIComponentTypeRegistry()
	: Data(nullptr)
{
	Data = new FUIComponentTypeRegistryData();

	// This will load the assets on next tick. It's not safe to do right now because we could be deep in a stack
	Data->Invalidate();

	IHotReloadInterface& HotReloadSupport = FModuleManager::LoadModuleChecked<IHotReloadInterface>("HotReload");
	HotReloadSupport.OnHotReload().AddRaw(this, &FUIComponentTypeRegistry::OnProjectHotReloaded);
}

FUIComponentTypeRegistry::~FUIComponentTypeRegistry()
{
	if( FModuleManager::Get().IsModuleLoaded("HotReload") )
	{
		IHotReloadInterface& HotReloadSupport = FModuleManager::GetModuleChecked<IHotReloadInterface>("HotReload");
		HotReloadSupport.OnHotReload().RemoveAll(this);
	}
}

bool FUIComponentTypeRegistry::IsClassABlueprintSpawnableComponent(const UClass* Class)
{
	return (!Class->HasAnyClassFlags(CLASS_Abstract) &&
		Class->IsChildOf<UActorComponent>() &&
		(Class->HasMetaData("UIBlueprintSpawnableComponent")));
}

void FUIComponentTypeRegistry::OnProjectHotReloaded( bool bWasTriggeredAutomatically )
{
	Data->ForceRefreshComponentList();
}

void FUIComponentTypeRegistry::InvalidateClass(TSubclassOf<UActorComponent> /*ClassToUpdate*/)
{
	Data->Invalidate();
}


#undef LOCTEXT_NAMESPACE
