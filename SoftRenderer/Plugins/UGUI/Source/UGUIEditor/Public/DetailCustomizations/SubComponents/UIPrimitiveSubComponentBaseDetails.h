#pragma once

#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

class UMaterialInterface;

class FUIPrimitiveSubComponentBaseDetails : public ISubComponentDetailCustomization
{
protected:
	virtual void AddUIPrimitiveDetails(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& OwnerCategory);

	/**
	 * Called by the material list widget when we need to get new materials for the list
	 *
	 * @param OutMaterials	Handle to a material list builder that materials should be added to
	 */
	virtual void OnGetMaterialsForView(class IMaterialListBuilder& OutMaterials);

	/**
	 * Called when a user drags a new material over a list item to replace it
	 *
	 * @param NewMaterial	The material that should replace the existing material
	 * @param PrevMaterial	The material that should be replaced
	 * @param SlotIndex		The index of the slot on the component where materials should be replaces
	 * @param bReplaceAll	If true all materials in the slot should be replaced not just ones using PrevMaterial
	 */
	virtual void OnMaterialChanged(UMaterialInterface* NewMaterial, UMaterialInterface* PrevMaterial, int32 SlotIndex, bool bReplaceAll);
	
protected:
	/** Notify hook to use */
	FNotifyHook* NotifyHook = nullptr;
	
	TWeakObjectPtr<class UBehaviourSubComponent> BehaviourSubComponentPtr;

};
