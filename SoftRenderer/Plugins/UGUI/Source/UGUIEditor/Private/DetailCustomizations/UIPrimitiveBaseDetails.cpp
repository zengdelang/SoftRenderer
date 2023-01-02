#include "DetailCustomizations/UIPrimitiveBaseDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "IPropertyUtilities.h"
#include "MaterialList.h"
#include "Core/BehaviourComponent.h"
#include "Core/Widgets/UIPrimitiveElementInterface.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

void FUIPrimitiveBaseDetails::AddUIPrimitiveDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& OwnerCategory)
{
	NotifyHook = DetailBuilder.GetPropertyUtilities()->GetNotifyHook();

	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	BehaviourComponentPtr = Cast<UBehaviourComponent>(TargetObjects[0].Get());

	if (BehaviourComponentPtr.IsValid())
	{
		IUIPrimitiveElementInterface* UIPrimitive = Cast<IUIPrimitiveElementInterface>(BehaviourComponentPtr.Get());
		if (UIPrimitive && UIPrimitive->GetNumOverrideMaterials())
		{
			FMaterialListDelegates MaterialListDelegates;
			MaterialListDelegates.OnGetMaterials.BindSP(this, &FUIPrimitiveBaseDetails::OnGetMaterialsForView);
			MaterialListDelegates.OnMaterialChanged.BindSP(this, &FUIPrimitiveBaseDetails::OnMaterialChanged);

			//Pass an empty material list owner (owner can be use by the asset picker filter. In this case we do not need it)
			TArray<FAssetData> MaterialListOwner;

			TSharedRef<FMaterialList> MaterialList = MakeShareable(new FMaterialList(DetailBuilder, MaterialListDelegates, MaterialListOwner));

			OwnerCategory.AddCustomBuilder(MaterialList);
		}
	}
}

void FUIPrimitiveBaseDetails::OnGetMaterialsForView(IMaterialListBuilder& MaterialList)
{
	const bool bAllowNullEntries = true;
	if (BehaviourComponentPtr.IsValid())
	{
		IUIPrimitiveElementInterface* UIPrimitive = Cast<IUIPrimitiveElementInterface>(BehaviourComponentPtr.Get());
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

void FUIPrimitiveBaseDetails::OnMaterialChanged(UMaterialInterface* NewMaterial, UMaterialInterface* PrevMaterial,
	int32 SlotIndex, bool bReplaceAll)
{
	if (BehaviourComponentPtr.IsValid())
	{
		IUIPrimitiveElementInterface* UIPrimitive = Cast<IUIPrimitiveElementInterface>(BehaviourComponentPtr.Get());
		if (!UIPrimitive)
		{
			return;
		}

		if (BehaviourComponentPtr->HasAnyFlags(RF_ArchetypeObject))
		{
			FProperty* MaterialProperty = FindFProperty<FProperty>(BehaviourComponentPtr->GetClass(), "OverrideMaterials");

			// Add a navigation update lock only if the component world is valid
			TSharedPtr<FNavigationLockContext> NavUpdateLock;
			UWorld* World = BehaviourComponentPtr->GetWorld();
			if (World)
			{
				NavUpdateLock = MakeShareable(new FNavigationLockContext(World, ENavigationLockReason::MaterialUpdate));
			}


			BehaviourComponentPtr->PreEditChange(MaterialProperty);
			if (NotifyHook && MaterialProperty)
			{
				NotifyHook->NotifyPreChange(MaterialProperty);
			}

			FPropertyChangedEvent PropertyChangedEvent(MaterialProperty);

			UIPrimitive->SetOverrideMaterial(SlotIndex, NewMaterial);

			if (!FApp::IsGame())
			{
				TArray<UObject*> ComponentArchetypeInstances;
				if (BehaviourComponentPtr->HasAnyFlags(RF_ArchetypeObject))
				{
					BehaviourComponentPtr->GetArchetypeInstances(ComponentArchetypeInstances);
				}
				else if (UObject* Outer = BehaviourComponentPtr->GetOuter())
				{
					TArray<UObject*> OuterArchetypeInstances;
					Outer->GetArchetypeInstances(OuterArchetypeInstances);
					for (auto OuterArchetypeInstance : OuterArchetypeInstances)
					{
						if (UObject* ArchetypeInstance = static_cast<UObject*>(FindObjectWithOuter(OuterArchetypeInstance, BehaviourComponentPtr->GetClass(), BehaviourComponentPtr->GetFName())))
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
					World = BehaviourComponentPtr->GetWorld();
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

			BehaviourComponentPtr->PostEditChangeProperty(PropertyChangeEvent);
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
