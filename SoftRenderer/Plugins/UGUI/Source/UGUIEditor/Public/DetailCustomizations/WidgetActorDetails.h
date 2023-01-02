#pragma once

#include "IDetailCustomization.h"

class UGUIEDITOR_API FWidgetActorDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;

	const FSlateBrush* GetImage() const ;

	void ApplyImage(const FString& ImagePath);

	TSharedPtr<FSlateDynamicImageBrush> LoadImageAsBrush(const FString& ImagePath);

	const FString RelativePathToAbsolutePath(const FString RelativePath);

	const FString GetRelativePath(const FString RawPath);

    bool DifferFromDefault()const;

private:
	TWeakObjectPtr<class AWidgetActor> TargetScriptPtr;

	TSharedPtr<FSlateDynamicImageBrush> ImageBrush;

};
