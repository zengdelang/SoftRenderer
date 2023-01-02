#pragma once

#include "CoreMinimal.h"
#include "SelectableComponent.h"
#include "EventSystem/Interfaces/CancelHandlerInterface.h"
#include "EventSystem/Interfaces/PointerClickHandlerInterface.h"
#include "EventSystem/Interfaces/SubmitHandlerInterface.h"
#include "Text/TextElementInterface.h"
#include "DropdownComponent.generated.h"

class AWidgetActor;

/**
 * Class to store the text and/or image of a single option in the dropdown list.
 */
USTRUCT(BlueprintType)
struct UGUI_API FOptionData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OptionData)
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OptionData, meta=(DisplayThumbnail="true", AllowedClasses="Sprite2D,SlateTextureAtlasInterface"))
	UObject* Image;

public:
	FOptionData() : Image(nullptr)
	{
	}

	FOptionData(FText InText) : Image(nullptr)
	{
		Text = InText;
	}

	FOptionData(UObject* InImage)
	{
		Image = InImage;
	}

	/**
	 * Create an object representing a single option for the dropdown list.
	 */
	FOptionData(FText InText, UObject* InImage)
	{
		Text = InText;
		Image = InImage;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDropdownEvent);

/**
 * A standard dropdown that presents a list of options when clicked, of which one can be chosen.
 *
 * The dropdown component is a Selectable. When an option is chosen, the label and/or image of the control changes to show the chosen option.
 *
 * When a dropdown event occurs a callback is sent to any registered listeners of onValueChanged.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Interaction), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UDropdownComponent : public USelectableComponent, public IPointerClickHandlerInterface, public ISubmitHandlerInterface, public ICancelHandlerInterface
{
	GENERATED_UCLASS_BODY()

	friend class UDropdownItemSubComponent;

protected:
	UPROPERTY(EditAnywhere, Category = Dropdown, meta=(DisplayName=Template))
	TSubclassOf<AWidgetActor> DropdownItemWidgetTemplateClass;
	
	/**
	 *  The list of possible options. A text string and an image can be specified for each option.
	 *
	 *  This is the list of options within the Dropdown. Each option contains Text and/or image data that you can specify using UI.Dropdown.OptionData before adding to the Dropdown list.
	 *  This also unlocks the ability to edit the Dropdown, including the insertion, removal, and finding of options, as well as other useful tools
	 */
	UPROPERTY(EditAnywhere, Category = Dropdown)
	TArray<FOptionData> Options;

	/**
	 * Text to be used as a caption for the current value. It's not required, but it's kept here for convenience.
	 * 
	 * The Text component to hold the text of the currently selected option.
	 */
	UPROPERTY(EditAnywhere, Category = Dropdown)
	TArray<int32> CaptionTextPath;

	/**
	 * The Image component to hold the image of the currently selected option.
	 */
	UPROPERTY(EditAnywhere, Category = Dropdown)
	TArray<int32> CaptionImagePath;
	
	/**
	 * The placeholder Graphic component. Shown when no option is selected.
	 */
	UPROPERTY(EditAnywhere, Category = Dropdown)
	TArray<int32> PlaceholderPath;

	UPROPERTY(EditAnywhere, Category = Dropdown)
	TArray<int32> DropdownPath;
	
	UPROPERTY(EditAnywhere, Category = Dropdown)
	TArray<int32> ScrollContentPath;

	UPROPERTY(EditAnywhere, Category = Dropdown)
	float ExtraSpace;
	
	/**
	 * The Value is the index number of the current selection in the Dropdown. 0 is the first option in the Dropdown, 1 is the second, and so on.
	 */
	UPROPERTY(EditAnywhere, Category = Dropdown)
	int32 Value;
	
	/**
	 * The time interval at which a drop down will appear and disappear
	 */
	UPROPERTY(EditAnywhere, Category = Dropdown)
	float AlphaFadeSpeed;

	FVector2D DropdownSize;
	
public:
	/**
	 * Notification triggered when the dropdown changes.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnDropdownEvent OnValueChanged;

protected:
	UPROPERTY(Transient)
	TScriptInterface<ITextElementInterface> CaptionText;

	UPROPERTY(Transient)
	TScriptInterface<IImageElementInterface> CaptionImage;
	
	UPROPERTY(Transient)
	TScriptInterface<IGraphicElementInterface> Placeholder;

	UPROPERTY(Transient)
	URectTransformComponent* ScrollContent;

	UPROPERTY(Transient)
	URectTransformComponent* Dropdown;

	UPROPERTY(Transient)
	TArray<class UDropdownItemSubComponent*> Items;
	
	TSharedPtr<FTweenRunner> AlphaTweenRunner;

	uint8 bValidTemplate : 1;
	uint8 bHasStarted : 1;
	uint8 bUpdateDropdownHeight : 1;
	uint8 bFlipAxisX : 1;
	uint8 bFlipAxisY : 1;
	uint8 bIsExpanded : 1;
	
	static FOptionData NoOptionData;
	
public:
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDestroy() override;

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
public:
	UFUNCTION(BlueprintCallable, Category = Dropdown)
	bool IsExpanded() const;
	
	UFUNCTION(BlueprintCallable, Category = Dropdown)
	float GetValue() const
	{
		return Value;
	}

	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void SetValue(int32 NewValue, bool bSendCallback = true)
	{
		if ((NewValue == Value || Options.Num() == 0))
		{
			if (bSendCallback && Options.Num() > 0)
			{
				OnValueChanged.Broadcast();
			}
			return;
		}
		
		Value = FMath::Clamp(NewValue, Placeholder ? -1 : 0, Options.Num() - 1);
		RefreshShownValue();

		if (bSendCallback)
		{
			OnValueChanged.Broadcast();
		}
	}

	/**
	 * Set index number of the current selection in the Dropdown without invoking onValueChanged callback.
	 *
	 * @param NewValue The new index for the current selection.
	 */
	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void SetValueWithoutNotify(int32 NewValue)
	{
		SetValue(NewValue, false);
	}
	
	UFUNCTION(BlueprintCallable, Category = Dropdown)
	float GetAlphaFadeSpeed() const
	{
		return AlphaFadeSpeed;
	}

	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void SetAlphaFadeSpeed(float NewAlphaFadeSpeed)
	{
		AlphaFadeSpeed = NewAlphaFadeSpeed;
	}

	UFUNCTION(BlueprintCallable, Category = Dropdown)
	TArray<FOptionData> GetOptions()
	{
		return Options;
	}
	
	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void SetOptions(const TArray<FOptionData>& InOptions)
	{
		Options = InOptions;
		RefreshShownValue();
	}

	void SetOptions(TArray<FOptionData>&& InOptions)
	{
		Options = MoveTemp(InOptions);
		RefreshShownValue();
	}

	UFUNCTION(BlueprintCallable, Category = Dropdown)
	TScriptInterface<ITextElementInterface> GetCaptionText()
	{
		return CaptionText;
	}

	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void SetCaptionText(TScriptInterface<ITextElementInterface> InCaptionText)
	{
		CaptionText = InCaptionText;
		RefreshShownValue();
	}
	
	UFUNCTION(BlueprintCallable, Category = Dropdown)
	TScriptInterface<IImageElementInterface> GetCaptionImage()
	{
		return CaptionImage;
	}

	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void SetCaptionImage(TScriptInterface<IImageElementInterface> InCaptionImage)
	{
		CaptionImage = InCaptionImage;
		RefreshShownValue();
	}

	UFUNCTION(BlueprintCallable, Category = Dropdown)
	TScriptInterface<IGraphicElementInterface> GetPlaceholder()
	{
		return Placeholder;
	}

	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void SetPlaceholder(TScriptInterface<IGraphicElementInterface> InPlaceholder)
	{
		Placeholder = InPlaceholder;
		RefreshShownValue();
	}

public:
	/**
	 * Refreshes the text and image (if available) of the currently selected option.
	 *
	 * If you have modified the list of options, you should call this method afterwards to ensure that the visual state of the dropdown corresponds to the updated options.
	 */
	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void RefreshShownValue();

	/**
	 * Add multiple options to the options of the Dropdown based on a list of OptionData objects.
	 */
	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void AddOptions(const TArray<FOptionData>& InOptions)
	{
		Options.Append(InOptions);
		RefreshShownValue();
	}

	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void AddOption(FOptionData InOption)
	{
		Options.Emplace(InOption);
		RefreshShownValue();
	}

	/***
	 * Add multiple text-only options to the options of the Dropdown based on a list of strings.
	 */
	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void AddOptionsByTexts(const TArray<FText>& InOptions)
	{
		for (int32 Index = 0, Count = InOptions.Num(); Index < Count; ++Index)
		{
			Options.Add(FOptionData(InOptions[Index]));
		}
		
		RefreshShownValue();
	}

	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void AddOptionByText(FText InOptionText)
	{
		Options.Add(FOptionData(InOptionText));
		RefreshShownValue();
	}

	/**
	 * Add multiple image-only options to the options of the Dropdown based on a list of Sprites.
	 */
	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void AddOptionsBySprites(const TArray<UObject*>& InOptions)
	{
		for (int32 Index = 0, Count = InOptions.Num(); Index < Count; ++Index)
		{
			Options.Add(FOptionData(InOptions[Index]));
		}
		
		RefreshShownValue();
	}

	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void AddOptionBySprite(UObject* InOptionSprite)
	{
		Options.Add(FOptionData(InOptionSprite));
		RefreshShownValue();
	}

	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void ClearOptions()
	{
		Options.Empty();
		Value = Placeholder ? -1 : 0;
		RefreshShownValue();
	}
	
public:
	/** Handling for when the dropdown is initially 'clicked'. Typically shows the dropdown */
	virtual void OnPointerClick(UPointerEventData* EventData) override;

	/** Handling for when the dropdown is selected and a submit event is processed. Typically shows the dropdown */
	virtual void OnSubmit(UBaseEventData* EventData) override;

	/**
	 * This will hide the dropdown list.
	 * Called by a BaseInputModule when a Cancel event occurs.
	 */
	virtual void OnCancel(UBaseEventData* EventData) override;

public:
	/**
	 * Show the dropdown.
	 *
	 * Plan for dropdown scrolling to ensure dropdown is contained within screen.
	 * We assume the Canvas is the screen that the dropdown must be kept inside.
	 * This is always valid for screen space canvas modes.
	 * For world space canvases we don't know how it's used, but it could be e.g. for an in-game monitor.
	 * We consider it a fair constraint that the canvas must be big enough to contain dropdowns.
	 */
	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void Show();

	UFUNCTION(BlueprintCallable, Category = Dropdown)
	void Hide();

protected:
	UCanvasSubComponent* GetRootCanvas() const;

	void SetupTemplate();

	UFUNCTION()
	void OnLostFocus();
	
	AWidgetActor* CreateItemWidget() const;
	void DestroyItemWidgets();

private:
	void AlphaFadeList(float Duration, float Start, float End, bool bDropdownEnabled);
	void SetAlpha(float Alpha) const;

	void OnSelectItem(int32 Index);
	
};
