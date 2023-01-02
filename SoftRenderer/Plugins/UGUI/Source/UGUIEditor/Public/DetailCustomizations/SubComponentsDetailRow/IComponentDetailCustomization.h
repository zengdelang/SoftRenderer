#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"

class IDetailCategoryBuilder;
class IDetailLayoutBuilder;

class UGUIEDITOR_API IComponentDetailCustomization : public IDetailCustomization
{
public:
	virtual ~IComponentDetailCustomization() override {}

public:
	virtual void AddRowHeaderContent(IDetailCategoryBuilder& CategoryBuilder, IDetailLayoutBuilder& DetailBuilder);

	virtual void AddEventProperty(IDetailCategoryBuilder& CategoryBuilder, IDetailLayoutBuilder& DetailBuilder, FName PropertyName);
	
	virtual FSlateBrush* GetIconBrush();

protected:
	FReply HandleAddOrViewEventForVariable(const FName EventName, FName VariableName, TWeakObjectPtr<UClass> PropertyClass);
	int32 HandleAddOrViewIndexForButton(const FName EventName, FName VariableName) const;
	
protected:
	TWeakObjectPtr<class UBehaviourComponent> BehaviourComponent;
	TWeakObjectPtr<UBlueprint> BlueprintObj;
	
};
