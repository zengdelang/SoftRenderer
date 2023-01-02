#pragma once

#include "CoreMinimal.h"
#include "UIEnumModifier.generated.h"

USTRUCT()
struct UGUI_API FUIEnumModifierItem
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FName Name;

	UPROPERTY()
	FName ToolTip;
};

USTRUCT()
struct UGUI_API FUIEnumModifier
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FUIEnumModifierItem> EnumItems;

	void SetEnumInfo(UEnum* EnumType) const
	{
#if WITH_EDITOR
		if (EnumType)
		{
			const FString KeyName = TEXT("DisplayName");
			const FString ToolTipKeyName = TEXT("ToolTip");
			const FString HiddenMeta = TEXT("Hidden");
			const FString UnusedDisplayName = TEXT("Unused");

			// remainders, set to be unused
			for (int32 EnumIndex = 1; EnumIndex < EnumType->NumEnums(); ++EnumIndex)
			{
				// if meta data isn't set yet, set name to "Unused" until we fix property window to handle this
				// make sure all hide and set unused first
				// if not hidden yet
				if (!EnumType->HasMetaData(*HiddenMeta, EnumIndex))
				{
					EnumType->SetMetaData(*HiddenMeta, TEXT(""), EnumIndex);
					EnumType->SetMetaData(*KeyName, *UnusedDisplayName, EnumIndex);
				}
			}

			int32 Type = 0;
			for (auto Iter = EnumItems.CreateConstIterator() + 1; Iter; ++Iter)
			{
				++Type;
				if (Iter->Name != NAME_None)
				{
					EnumType->SetMetaData(*KeyName, *Iter->Name.ToString(), Type);

					if (Iter->ToolTip == NAME_None)
					{
						EnumType->SetMetaData(*ToolTipKeyName, TEXT(""), Type);
					}
					else
					{
						EnumType->SetMetaData(*ToolTipKeyName, *Iter->ToolTip.ToString(), Type);
					}

					// also need to remove "Hidden"
					EnumType->RemoveMetaData(*HiddenMeta, Type);
				}
			}
		}
#endif
	}
	
};
