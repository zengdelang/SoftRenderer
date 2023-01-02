#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "Core/BehaviourSubComponent.h"

class IDetailCategoryBuilder;
class IDetailLayoutBuilder;

class UGUIEDITOR_API ISubComponentDetailCustomization : public TSharedFromThis<ISubComponentDetailCustomization>
{
public:
	virtual ~ISubComponentDetailCustomization() {}

	static TSharedRef<ISubComponentDetailCustomization> MakeInstance();

	virtual void SetSubComponent(UBehaviourSubComponent* InSubComponent, UObject* InOwnerObject, FName InCategoryName, 
		int32 InSubIndex, const TAttribute<bool>& InIsParentEnabled);

	virtual void AddRowHeaderContent(IDetailCategoryBuilder& CategoryBuilder, IDetailLayoutBuilder& DetailBuilder);
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& CategoryBuilder) {}

	IDetailPropertyRow* AddProperty(FName PropertyName, IDetailCategoryBuilder& CategoryBuilder, EPropertyLocation::Type Location = EPropertyLocation::Default) const;

	FSlateBrush* GetIconBrush();
	
protected:
	TWeakObjectPtr<UBehaviourSubComponent> SubComponent;

	TWeakObjectPtr<UObject> OwnerObject;

	FName CategoryName;

	int32 SubIndex = 0;

	/** Whether or not our parent is enabled */
	TAttribute<bool> IsParentEnabled;
};
