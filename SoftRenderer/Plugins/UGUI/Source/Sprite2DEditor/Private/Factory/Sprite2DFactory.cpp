#include "Sprite2DFactory.h"
#include "AssetToolsModule.h"
#include "Sprite2D.h"

#define LOCTEXT_NAMESPACE "Sprite2DFactory"

USprite2DFactory::USprite2DFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = USprite2D::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
	InitialTexture = nullptr;
}

UObject* USprite2DFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	USprite2D* Asset = NewObject<USprite2D>(InParent, InName, Flags | RF_Transactional);
	if (InitialTexture)
	{
		Asset->SetSourceTextureForEditor(InitialTexture);
		Asset->SetSourceDimensionForEditor(FVector2D(InitialTexture->GetSizeX(),InitialTexture->GetSizeY()));
	}
	return Asset;
}

#undef LOCTEXT_NAMESPACE