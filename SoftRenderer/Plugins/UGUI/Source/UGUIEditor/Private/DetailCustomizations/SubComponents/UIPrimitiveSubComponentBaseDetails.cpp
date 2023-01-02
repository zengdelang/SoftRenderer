#include "DetailCustomizations/SubComponents/UIPrimitiveSubComponentBaseDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "MaterialList.h"
#include "Core/Widgets/UIPrimitiveElementInterface.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

void FUIPrimitiveSubComponentBaseDetails::AddUIPrimitiveDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	BehaviourSubComponentPtr = Cast<UBehaviourSubComponent>(SubComponent.Get());

	if (BehaviourSubComponentPtr.IsValid())
	{
		IUIPrimitiveElementInterface* UIPrimitive = Cast<IUIPrimitiveElementInterface>(BehaviourSubComponentPtr.Get());
		if (UIPrimitive && UIPrimitive->GetNumOverrideMaterials())
		{
			FMaterialListDelegates MaterialListDelegates;
			MaterialListDelegates.OnGetMaterials.BindSP(this, &FUIPrimitiveSubComponentBaseDetails::OnGetMaterialsForView);
			MaterialListDelegates.OnMaterialChanged.BindSP(this, &FUIPrimitiveSubComponentBaseDetails::OnMaterialChanged);

			//Pass an empty material list owner (owner can be use by the asset picker filter. In this case we do not need it)
			TArray<FAssetData> MaterialListOwner;

			TSharedRef<FMaterialList> MaterialList = MakeShareable(new FMaterialList(DetailBuilder, MaterialListDelegates, MaterialListOwner));

			CategoryBuilder.AddCustomBuilder(MaterialList);
		}
	}
}

void FUIPrimitiveSubComponentBaseDetails::OnGetMaterialsForView(IMaterialListBuilder& MaterialList)
{
	const bool bAllowNullEntries = true;
	if (BehaviourSubComponentPtr.IsValid())
	{
		IUIPrimitiveElementInterface* UIPrimitive = Cast<IUIPrimitiveElementInterface>(BehaviourSubComponentPtr.Get());
		if (UIPrimitive)
		{
			for (int32 Index = 0, Count = UIPrimitive->GetNumOverrideMaterials(); Index < Count; ++Index)
			{
				UMaterialInterface* Material = UIPrimitive->GetOverrideMaterial(Index);
				if (bAllowNullEntries || Material)
				{
					MaterialList.AddMaterial(Index, Material, true);
				}
			}
		}
	}
}

void FUIPrimitiveSubComponentBaseDetails::OnMaterialChanged(UMaterialInterface* NewMaterial, UMaterialInterface* PrevMaterial,
	int32 SlotIndex, bool bReplaceAll)
{
	if (BehaviourSubComponentPtr.IsValid())
	{
		IUIPrimitiveElementInterface* UIPrimitive = Cast<IUIPrimitiveElementInterface>(BehaviourSubComponentPtr.Get());
		if (!UIPrimitive)
		{
			return;
		}
		
		if (BehaviourSubComponentPtr->HasAnyFlags(RF_ArchetypeObject))
		{
			FProperty* MaterialProperty = FindFProperty<FProperty>(BehaviourSubComponentPtr->GetClass(), "OverrideMaterials");

			// Add a navigation update lock only if the component world is valid
			TSharedPtr<FNavigationLockContext> NavUpdateLock;
			UWorld* World = BehaviourSubComponentPtr->GetWorld();
			if (World)
			{
				NavUpdateLock = MakeShareable(new FNavigationLockContext(World, ENavigationLockReason::MaterialUpdate));
			}


			BehaviourSubComponentPtr->PreEditChange(MaterialProperty);
			if (NotifyHook && MaterialProperty)
			{
				NotifyHook->NotifyPreChange(MaterialProperty);
			}

			FPropertyChangedEvent PropertyChangedEvent(MaterialProperty);

			UIPrimitive->SetOverrideMaterial(SlotIndex, NewMaterial);

			if (!FApp::IsGame())
			{
				TArray<UObject*> ComponentArchetypeInstances;
				if (BehaviourSubComponentPtr->HasAnyFlags(RF_ArchetypeObject))
				{
					BehaviourSubComponentPtr->GetArchetypeInstances(ComponentArchetypeInstances);
				}
				else if (UObject* Outer = BehaviourSubComponentPtr->GetOuter())
				{
					TArray<UObject*> OuterArchetypeInstances;
					Outer->GetArchetypeInstances(OuterArchetypeInstances);
					for (auto OuterArchetypeInstance : OuterArchetypeInstances)
					{
						if (UObject* ArchetypeInstance = static_cast<UObject*>(FindObjectWithOuter(OuterArchetypeInstance, BehaviourSubComponentPtr->GetClass(), BehaviourSubComponentPtr->GetFName())))
						{
							ComponentArchetypeInstances.Add(ArchetypeInstance);
						}
					}
				}

				IUIPrimitiveElementInterface* CurrentComponent = nullptr;
				for (auto ComponentArchetypeInstance : ComponentArchetypeInstances)
				{
					CurrentComponent = CastChecked<IUIPrimitiveElementInterface>(ComponentArchetypeInstance);

					// Reset the navigation update lock if necessary
					UWorld* PreviousWorld = World;
					World = BehaviourSubComponentPtr->GetWorld();
					if (PreviousWorld != World)
					{
						NavUpdateLock = MakeShareable(new FNavigationLockContext(World, ENavigationLockReason::MaterialUpdate));
					}

					ComponentArchetypeInstance->PreEditChange(MaterialProperty);

					CurrentComponent->SetOverrideMaterial(SlotIndex, NewMaterial);

					ComponentArchetypeInstance->PostEditChangeProperty(PropertyChangedEvent);
				}
			}

			FPropertyChangedEvent PropertyChangeEvent(MaterialProperty, EPropertyChangeType::ValueSet);

			BehaviourSubComponentPtr->PostEditChangeProperty(PropertyChangeEvent);
			if (NotifyHook && MaterialProperty)
			{
				NotifyHook->NotifyPostChange(PropertyChangeEvent, MaterialProperty);
			}
		}
		else
		{
			UIPrimitive->SetOverrideMaterial(SlotIndex, NewMaterial);
		}
	}
}

#undef LOCTEXT_NAMESPACE
