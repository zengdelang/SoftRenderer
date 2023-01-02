#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/SSubComponentDetailCategory.h"
#include "Styling/SlateIconFinder.h"

TSharedRef<ISubComponentDetailCustomization> ISubComponentDetailCustomization::MakeInstance()
{
	return MakeShareable(new ISubComponentDetailCustomization);
}

void ISubComponentDetailCustomization::SetSubComponent(UBehaviourSubComponent* InSubComponent, UObject* InOwnerObject, 
                                                       FName InCategoryName, int32 InSubIndex, const TAttribute<bool>& InIsParentEnabled)
{
	SubComponent = InSubComponent;
	CategoryName = InCategoryName;
	SubIndex = InSubIndex;
	OwnerObject = InOwnerObject;
	IsParentEnabled = InIsParentEnabled;
}

void ISubComponentDetailCustomization::AddRowHeaderContent(IDetailCategoryBuilder& CategoryBuilder, IDetailLayoutBuilder& DetailBuilder)
{
	const auto EnabledRow = AddProperty(TEXT("bIsEnabled"), CategoryBuilder);
	if (EnabledRow)
	{
		EnabledRow->Visibility(EVisibility::Collapsed);
		auto EnabledHandle = EnabledRow->GetPropertyHandle();
		const auto RefreshPropertyHandle = DetailBuilder.GetProperty(TEXT("bRefreshDetailForEditor"));
		const auto SubComponentsPropertyHandle = DetailBuilder.GetProperty(TEXT("SubComponents"));

		bool bHideEnableCheckBox = false;
		if (SubComponent.IsValid())
		{
			bHideEnableCheckBox = SubComponent->GetClass()->GetBoolMetaData(TEXT("HideEnableCheckBox"));
		}
		
		const TSharedRef<SSubComponentDetailCategory> DetailCategory = SNew(SSubComponentDetailCategory, EnabledHandle,
			RefreshPropertyHandle, SubComponentsPropertyHandle, OwnerObject.Get(), IsParentEnabled);
		DetailCategory->bHideEnableCheckBox = bHideEnableCheckBox;
		DetailCategory->ComponentIcon = GetIconBrush();
		DetailCategory->SetIndex(SubIndex);
		CategoryBuilder.HeaderContent(DetailCategory);
	}
}

IDetailPropertyRow* ISubComponentDetailCustomization::AddProperty(FName PropertyName, IDetailCategoryBuilder& CategoryBuilder, EPropertyLocation::Type Location) const
{
	TArray<UObject*> Objects;
	Objects.Add(SubComponent.Get());
	return CategoryBuilder.AddExternalObjectProperty(Objects, PropertyName, Location);
}

FSlateBrush* ISubComponentDetailCustomization::GetIconBrush()
{
	FSlateBrush* ComponentIcon = const_cast<FSlateBrush*>(FEditorStyle::GetBrush("SCS.NativeComponent"));
	if (UBehaviourSubComponent* ComponentTemplate = SubComponent.Get())
	{
		ComponentIcon = const_cast<FSlateBrush*>(FSlateIconFinder::FindIconBrushForClass(ComponentTemplate->GetClass(), TEXT("SCS.Component")));
	}
	return ComponentIcon;
}