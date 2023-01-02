#pragma once

#include "CoreMinimal.h"
#include "SelectableComponent.h"
#include "Core/Widgets/Text/TextSubComponent.h"
#include "Core/CanvasUpdateRegistry.h"
#include "Core/Render/LateUpdateInterface.h"
#include "EventSystem/InputModules/BaseInput.h"
#include "EventSystem/InputModules/BaseInputModuleSubComponent.h"
#include "EventSystem/Interfaces/BeginDragHandlerInterface.h"
#include "EventSystem/Interfaces/DragHandlerInterface.h"
#include "EventSystem/Interfaces/EndDragHandlerInterface.h"
#include "EventSystem/Interfaces/PointerClickHandlerInterface.h"
#include "EventSystem/Interfaces/SubmitHandlerInterface.h"
#include "EventSystem/Interfaces/UpdateSelectedHandlerInterface.h"
#include "GenericPlatform/ITextInputMethodSystem.h"
#include "HAL/PlatformApplicationMisc.h"
#include "InputFieldComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE_RetVal_ThreeParams(int32, FInputFieldOnValidateInput, const FString&, TextString, int32, CharIndex, int32, AddedChar);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputFieldSubmitEvent, FText, Text);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputFieldOnChangeEvent, FText, Text);

/**
 * Setting the content type acts as a shortcut for setting a combination of InputType, CharacterValidation, LineType, and TouchScreenKeyboardType
 *
 * The ContentType affects character validation, keyboard type used (on platforms with on-screen keyboards), whether the InputField accepts multiple lines, and whether the text is autocorrected (on platforms that offer input auto-correction) or is treated as a password where the characters are not shown directly.
 */
UENUM()
enum class EInputFieldContentType : uint8
{
	/**
	 * Allows all input.
	 */
	Standard,

	/**
	 * Allows all input and performs auto-correction on platforms that support it.
	 */
	AutoCorrected,

	/**
	 * Allow whole numbers (positive or negative).
	 */
	IntegerNumber,

	/**
	 * Allows decimal numbers (positive or negative).
	 */
	DecimalNumber,

	/**
	 * Allows letters A-Z, a-z and numbers 0-9.
	 */
	Alphanumeric,

	/**
	 * The InputField is used for typing in a name, and enforces capitalization of the first letter of each word. Note that the user can circumvent the first letter capitalization rules by deleting automatically-capitalized letters.
	 */
	Name,

	/**
	 * The input is used for typing in an email address.
	 */
	EmailAddress,

	/**
	 * Allows all input and hides the typed characters by showing them as asterisks characters.
	 */
	Password,

	/**
	 * Allows integer numbers and hides the typed characters by showing them as asterisks characters.
	 */
	Pin,

	/**
     * Custom types that allows user-defined settings.
	 */
	Custom,
};

/**
 * Type of data expected by the input field mobile keyboard.
 */
UENUM()
enum class EInputFieldInputType : uint8
{
	/**
	 * The standard mobile keyboard
	 */
	Standard,

	/**
	 * The mobile autocorrect keyboard.
	 */
	AutoCorrect,

	/**
 	 * The mobile password keyboard.
 	 */
	Password,

};

/**
 * The type of characters that are allowed to be added to the string.
 *
 * Note that the character validation does not validate the entire string as being valid or not. It only does validation on a per-character level, resulting in the typed character either being added to the string or not
 */
UENUM()
enum class EInputFieldCharacterValidation : uint8
{
	/**
	 * No validation. Any input is valid.
	 */
	None,

	/**
	 * Allow whole numbers (positive or negative).
	 * Characters 0-9 and - (dash / minus sign) are allowed. The dash is only allowed as the first character.
	 */
	Integer,

	/**
	 * Allows decimal numbers (positive or negative).
	 * Characters 0-9, . (dot), and - (dash / minus sign) are allowed. The dash is only allowed as the first character. Only one dot in the string is allowed.
	 */
	Decimal,
	
	/**
	 * Allows letters A-Z, a-z and numbers 0-9.
	 */
	Alphanumeric,

	/**
	 * Only allow names and enforces capitalization.
	 *
	 * Allows letters, spaces, and ' (apostrophe). A character after a space is automatically made upper case. A character not after a space is automatically made lowercase. A character after an apostrophe can be either upper or lower case. Only one apostrophe in the string is allowed. More than one space in a row is not allowed.
     * A characters is considered a letter if it is categorized as a Unicode letter.
	 */
	Name,

	/**
	 * Allows the characters that are allowed in an email address.
	 * 
	 * Allows characters A-Z, a.z, 0-9, @, . (dot), !, #, $, %, &amp;, ', *, +, -, /, =, ?, ^, _, `, {, |, }, and ~.
	 * Only one @ is allowed in the string and more than one dot in a row are not allowed. Note that the character validation does not validate the entire string as being a valid email address since it only does validation on a per-character level, resulting in the typed character either being added to the string or not.
	 */
	EmailAddress,
};

/**
 * The LineType is used to describe the behavior of the InputField.
 */
UENUM()
enum class EInputFieldLineType : uint8
{
	/**
	 * Only allows 1 line to be entered. Has horizontal scrolling and no word wrap. Pressing enter will submit the InputField.
	 */
	SingleLine,

	/**
	 * Is a multiline InputField with vertical scrolling and overflow. Pressing the return key will submit.
	 */
	MultiLineSubmit,

	/**
	 * Is a multiline InputField with vertical scrolling and overflow. Pressing the return key will insert a new line character.
	 */
	MultiLineNewline,

};

UENUM()
enum class ETouchScreenKeyboardType : uint8
{
	Keyboard_Default,
	Keyboard_Number,
	Keyboard_Web,
	Keyboard_Email,
	Keyboard_Password,
	Keyboard_AlphaNumeric,
};

/**
 * Turn a simple label into a interactable input field.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Interaction), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UInputFieldComponent : public USelectableComponent, public IUpdateSelectedHandlerInterface, public IBeginDragHandlerInterface,
		public IDragHandlerInterface, public IEndDragHandlerInterface, public IPointerClickHandlerInterface, public ISubmitHandlerInterface,
		public ICanvasElementInterface, public ILayoutElementInterface, public ILateUpdateInterface
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(Transient)
	TScriptInterface<ITextElementInterface> TextComponent;
	
	UPROPERTY(Transient)
	UBehaviourComponent* Placeholder;

	UPROPERTY(EditAnywhere, Category = InputField)
	TArray<int32> TextComponentPath;

	UPROPERTY(EditAnywhere, Category = InputField)
	TArray<int32> PlaceholderPath;
	
private:
	UPROPERTY(EditAnywhere, Category = InputField)
	EInputFieldContentType ContentType;

	UPROPERTY(EditAnywhere, Category = InputField)
	EInputFieldInputType InputType;

	TCHAR AsteriskChar;

	UPROPERTY(EditAnywhere, Category = InputField)
	ETouchScreenKeyboardType KeyboardType;

	UPROPERTY(EditAnywhere, Category = InputField)
	EInputFieldLineType LineType;

	UPROPERTY(EditAnywhere, Category = InputField)
	EInputFieldCharacterValidation CharacterValidation;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", IMin = "0"), Category = InputField)
	int32 CharacterLimit;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (IsBindableEvent = "True"), Category = InputField)
	FInputFieldOnValidateInput OnValidateInput;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FInputFieldSubmitEvent OnEndEdit;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FInputFieldOnChangeEvent OnValueChanged;

private:
	UPROPERTY(EditAnywhere, Category = InputField)
	FLinearColor CaretColor;
	
	UPROPERTY(EditAnywhere, Category = InputField)
	FLinearColor SelectionColor;

protected:
	UPROPERTY(EditAnywhere, Category = InputField)
	FText Text;

private:
	FText OriginalText;
	
	FDelegateHandle MarkGeometryAsDirtyHandle;
	FDelegateHandle UpdateLabelHandle;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "4", UIMin = "0", UIMax = "4"), Category = InputField)
	float CaretBlinkRate;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", ClampMax = "5", UIMin = "1", UIMax = "5"), Category = InputField)
	int32 CaretWidth;

	UPROPERTY(EditAnywhere, Category = InputField)
	uint8 bReadOnly : 1;

	UPROPERTY(EditAnywhere, Category = InputField)
	uint8 bCustomCaretColor : 1;
	
	UPROPERTY(EditAnywhere, Category = InputField)
	uint8 bHideMobileInput : 1;

	uint8 bPreventFontCallback : 1;
	uint8 bAllowInput : 1;
	uint8 bShouldActivateNextUpdate : 1;
	uint8 bUpdateDrag : 1;
	uint8 bDragPositionOutOfBounds : 1;

protected:
	uint8 bCaretVisible : 1;
	uint8 bNeedSetTimer : 1;
	
private:
	uint8 bWasCanceled : 1;
	uint8 bHasDoneFocusTransition : 1;
	uint8 bTouchKeyboardAllowsInPlaceEditing : 1;
	uint8 bHasRegisteredTextInputMethodContext : 1;
	uint8 bUseIntegratedKeyboard : 1;
	
protected:
	int32 CaretPosition;
	int32 CaretSelectPosition;

	TWeakObjectPtr<URectTransformComponent> CaretRectTrans;

	TArray<FUIVertex> CursorVerts;
	TSharedPtr<FTextGenerator> InputTextCache;
	TWeakObjectPtr<UCanvasRendererSubComponent> CachedInputRenderer;

	int32 DrawStart;
	int32 DrawEnd;

	FTimerHandle BlinkTimerHandle;
	float BlinkStartTime;

	FString CompositionString;
	
private:
	FORCEINLINE IUGUIInputInterface* GetInput() const
	{
		const auto World = GetWorld();
		const auto EventSystem = UEventSystemComponent::GetCurrentEventSystem(World);
		if (IsValid(EventSystem) && EventSystem->GetCurrentInputModule())
			return EventSystem->GetCurrentInputModule()->GetInput();
		return nullptr;
	}

	const FString& GetCompositionString() const
	{
		return CompositionString;
	}

public:
	/**
	 * Does the InputField currently have focus and is able to process events.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	bool IsFocused() const
	{
		return bAllowInput;
	}

	/**
	 * If the input field supports multiple lines.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	bool IsMultiLine() const
	{
		return LineType == EInputFieldLineType::MultiLineNewline || LineType == EInputFieldLineType::MultiLineSubmit;
	}

	/**
	* The beginning point of the selection.
	*
	* When making a selection with a mouse, the anchor is where in the document the mouse button is initially pressed.
	* Returns the beginning position of selection
	*/
	UFUNCTION(BlueprintCallable, Category = InputField)
	int32 GetSelectionAnchorPosition()
	{
		return CaretPosition + GetCompositionString().Len();
	}

	/**
	* If Input.compositionString is 0 set the fixed position.
	*/
	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetSelectionAnchorPosition(int32 InSelectPos)
	{
		if (GetCompositionString().Len() != 0)
			return;

		CaretPosition = InSelectPos;
		ClampPos(CaretPosition);
	}

	/**
	 * The end point of the selection.
	 *
	 * When making a selection with a mouse, the focus is where in the document the mouse button is released.
	 * Returns the end position of selection
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	int32 GetSelectionFocusPosition()
	{
		return CaretSelectPosition + GetCompositionString().Len();
	}

	/**
	 * If Input.compositionString is 0 set the variable position.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetSelectionFocusPosition(int32 InSelectPos)
	{
		if (GetCompositionString().Len() != 0)
			return;

		CaretSelectPosition = InSelectPos;
		ClampPos(CaretSelectPosition);
	}

	/**
	 * Helper function to allow separate events to be processed by the InputField.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	void ProcessKeyEvent(FKeyCharacterEvent Event)
	{
		KeyPressed(Event);
	}

	/**
	 * Force the label to update immediately. This will recalculate the positioning of the caret and the visible text.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	void ForceLabelUpdate()
	{
		UpdateLabel();
	}

	/**
	* Should the mobile keyboard input be hidden. This allows for input to happen with a caret in the InputField instead of a OS input box above the keyboard.
	*/
	UFUNCTION(BlueprintCallable, Category = InputField)
	bool ShouldHideMobileInput() const
	{
#if PLATFORM_TVOS || PLATFORM_ANDROID_ARM || PLATFORM_ANDROID || PLATFORM_ANDROID_ARM64 || PLATFORM_ANDROID_X86 || PLATFORM_ANDROID_X64
		return bHideMobileInput;
#else
		return true;
#endif
	}

	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetShouldHideMobileInput(bool bInHideMobileInput)
	{
		bHideMobileInput = bInHideMobileInput;
	}
	
	UFUNCTION(BlueprintCallable, Category = InputField)
	bool ShouldActivateOnSelect() const
	{
#if PLATFORM_TVOS
		return false;
#else
		return true;
#endif
	}

	/**
	 * Input field's current text value. This is not necessarily the same as what is visible on screen.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	FText GetText() const
	{
		return Text;
	}

	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetText(FText Value)
	{
		SetText(Value.ToString());
	}
	
	/**
	 * The blinking rate of the input caret, defined as the number of times the blink cycle occurs per second.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	float GetCaretBlinkRate() const
	{
		return CaretBlinkRate;
	}

	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetCaretBlinkRate(float NewCaretBlinkRate)
	{
		if (CaretBlinkRate != NewCaretBlinkRate)
		{
			CaretBlinkRate = NewCaretBlinkRate;
			if (bAllowInput)
			{
				SetCaretActive();
			}
		}
	}
	
	/**
	 * The width of the caret in pixels.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	int32 GetCaretWidth() const
	{
		return CaretWidth;
	}

	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetCaretWidth(int32 NewCaretWidth)
	{
		if (CaretWidth != NewCaretWidth)
		{
			CaretWidth = NewCaretWidth;
			MarkGeometryAsDirty();
		}
	}

	/**
	* The custom caret color used if customCaretColor is set.
	*/
	UFUNCTION(BlueprintCallable, Category = InputField)
	FLinearColor GetCaretColor() const
	{
		if (bCustomCaretColor)
		{
			return CaretColor;
		}

		const auto GraphicElem = Cast<IGraphicElementInterface>(TextComponent.GetObject());
		if (GraphicElem)
		{
			return GraphicElem->GetColor();
		}
		return CaretColor;
	}

	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetCaretColor(FLinearColor NewColor)
	{
		if (CaretColor != NewColor)
		{
			CaretColor = NewColor;
			MarkGeometryAsDirty();
		}
	}

	/**
	* Should a custom caret color be used or should the textComponent.color be used.
	*/
	UFUNCTION(BlueprintCallable, Category = InputField)
	bool IsCustomCaretColor() const
	{
		return bCustomCaretColor;
	}

	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetCustomCaretColor(bool bInCustomCaretColor)
	{
		if (bCustomCaretColor != bInCustomCaretColor)
		{
			bCustomCaretColor = bInCustomCaretColor;
			MarkGeometryAsDirty();
		}
	}

	/**
	 * The color of the highlight to show which characters are selected.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	FLinearColor GetSelectionColor() const
	{
		return SelectionColor;
	}

	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetSelectionColor(FLinearColor NewColor)
	{
		if (SelectionColor != NewColor)
		{
			SelectionColor = NewColor;
			MarkGeometryAsDirty();
		}
	}
	
	/**
	 * How many characters the input field is limited to. 0 = infinite.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	int32 GetCharacterLimit() const
	{
		return CharacterLimit;
	}

	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetCharacterLimit(int32 NewCharacterLimit)
	{
		NewCharacterLimit = NewCharacterLimit =FMath::Max(0, NewCharacterLimit);
		if (CharacterLimit != NewCharacterLimit)
		{
			CharacterLimit = NewCharacterLimit;
			UpdateLabel();
		}
	}

	/**
	* Specifies the type of the input text content.
	*
	* The ContentType affects character validation, keyboard type used (on platforms with on-screen keyboards), whether the InputField accepts multiple lines, and whether the text is autocorrected (on platforms that offer input auto-correction) or is treated as a password where the characters are not shown directly.
	*/
	UFUNCTION(BlueprintCallable, Category = InputField)
	EInputFieldLineType GetLineType() const
	{
		return LineType;
	}

	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetLineType(EInputFieldLineType NewLineType)
	{
		if (LineType != NewLineType)
		{
			LineType = NewLineType;
			TArray<EInputFieldContentType, TInlineAllocator<8>> AllowedContentType;
			AllowedContentType.Emplace(EInputFieldContentType::Standard);
			AllowedContentType.Emplace(EInputFieldContentType::AutoCorrected);
			SetToCustomIfContentTypeIsNot(AllowedContentType);
			EnforceTextHOverflow();
		}
	}

	UFUNCTION(BlueprintCallable, Category = InputField)
	EInputFieldContentType GetContentType() const
	{
		return ContentType;
	}
	
	/**
	* The type of input expected. See InputField.InputType.
	*/
	UFUNCTION(BlueprintCallable, Category = InputField)
	EInputFieldInputType GetInputType() const
	{
		return InputType;
	}

	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetInputType(EInputFieldInputType NewInputType)
	{
		if (InputType != NewInputType)
		{
			InputType = NewInputType;
			SetToCustom();
		}
	}

	/**
	 * They type of mobile keyboard that will be used.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	ETouchScreenKeyboardType GetKeyboardType() const
	{
		return KeyboardType;
	}

	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetKeyboardType(ETouchScreenKeyboardType InKeyboardType)
	{
		if (KeyboardType != InKeyboardType)
		{
			KeyboardType = InKeyboardType;
			SetToCustom();
		}
	}

	/**
	 * The type of validation to perform on a character
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	EInputFieldCharacterValidation GetCharacterValidation() const
	{
		return CharacterValidation;
	}

	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetCharacterValidation(EInputFieldCharacterValidation NewCharacterValidation)
	{
		if (CharacterValidation != NewCharacterValidation)
		{
			CharacterValidation = NewCharacterValidation;
			SetToCustom();
		}
	}
	
	/**
	* The character used to hide text in password field.
	*/
	UFUNCTION(BlueprintCallable, Category = InputField)
	bool IsReadOnly() const
	{
		return bReadOnly;
	}

	/**
	 * Set the InputField to be read only.
	 *
	 * Setting read only allows for highlighting of text without allowing modifications via keyboard.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetReadOnly(bool bInReadOnly)
	{
		bReadOnly = bInReadOnly;
	}

	/**
	 * The character used to hide text in password field.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	int32 GetAsteriskChar() const
	{
		return AsteriskChar;
	}

	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetAsteriskChar(int32 CharCode)
	{
		if (AsteriskChar != CharCode)
		{
			AsteriskChar = CharCode;
			UpdateLabel();
		}
	}
	
	/**
	 * If the InputField was canceled and will revert back to the original text upon DeactivateInputField.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	bool WasCanceled() const
	{
		return bWasCanceled;
	}
	
	/**
	 * Returns the focus position as thats the position that moves around even during selection.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	int32 GetCaretPosition() const
	{
		return CaretSelectPosition + GetCompositionString().Len();
	}

	/**
	* Set both the anchor and focus position such that a selection doesn't happen
	*/
	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetCaretPosition(int32 NewPos)
	{
		SetSelectionAnchorPosition(NewPos);
		SetSelectionFocusPosition(NewPos);
	}

	/**
	 * Set Input field's current text value without invoke onValueChanged. This is not necessarily the same as what is visible on screen.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	void SetTextWithoutNotify(FText Input)
	{
		SetText(Input.ToString(), false);
	}
	
protected:
	TSharedPtr<FTextGenerator>& GetCachedInputTextGenerator()
	{
		if (!InputTextCache.IsValid())
		{
			InputTextCache = MakeShareable(new FTextGenerator());
			InputTextCache->bGenerateCharacterAndLineInfo = true;
		}
		return InputTextCache;
	}
	
	FORCEINLINE void ClampPos(int32& Pos) const
	{
		if (Pos < 0)
		{
			Pos = 0;
		}
		else
		{
			const FString TextString = Text.ToString();
			if (Pos > TextString.Len())
			{
				Pos = TextString.Len();
			}
		}
	}

	FORCEINLINE int32 GetCaretPositionInternal() const
	{
		return CaretPosition + GetCompositionString().Len();
	}

	FORCEINLINE void SetCaretPositionInternal(int32 InCaretPos)
	{
		CaretPosition = InCaretPos;
		ClampPos(CaretPosition);
	}

	FORCEINLINE int32 GetCaretSelectPositionInternal() const
	{
		return CaretSelectPosition + GetCompositionString().Len();
	}

	FORCEINLINE void SetCaretSelectPositionInternal(int32 InCaretSelectPos)
	{
		CaretSelectPosition = InCaretSelectPos;
		ClampPos(CaretSelectPosition); 
	}
	
private:
	FORCEINLINE bool HasSelection() const
	{
		return GetCaretPositionInternal() != GetCaretSelectPositionInternal();
	}
	
	FORCEINLINE bool InPlaceEditing() const
	{
		return bTouchKeyboardAllowsInPlaceEditing || !FPlatformApplicationMisc::RequiresVirtualKeyboard();
	}

	FORCEINLINE bool IsValidChar(TCHAR Character) const
	{
		// Delete key on mac
		if (Character == 127)
			return false;
		
		// Accept newline and tab
		if (Character == TEXT('\t') || Character == TEXT('\n'))
			return true;

		if (Character < TEXT(' '))
		{
			return false;
		}

		if (TextComponent)
		{
			const auto Font = TextComponent->GetFont();
			if (Font)
			{
				bool bUseNormalStyle = true;
				const TCHAR Char = Font->RemapChar(Character, bUseNormalStyle);
				if (Font->Characters.IsValidIndex(Char))
				{
					const auto& FontChar = Font->Characters[Char];
					return FontChar.USize > 0 || FontChar.Advance > 0;
				}
			}
		}
		return false;
	}

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
	void UpdateEditChange();
#endif
	
	//~ Begin BehaviourComponent Interface
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void OnDestroy() override;
	//~ End BehaviourComponent Interface.

	virtual void LateUpdate() override;

private:
	void SetText(FString Value, bool  bSendCallback = true);
	
protected:
	void CaretBlink();
	
	void SetCaretVisible();

	/**
	 * SetCaretActive will not set the caret immediately visible - it will wait for the next time to blink.
	 * However, it will handle things correctly if the blink speed changed from zero to non-zero or non-zero to zero.
	 */
	void SetCaretActive();
	
	/**
	 * Focus the input field initializing properties.
	 *
	 * Handles what happens after a user selects an InputField. This is a protected property. To return the focus state use InputField.isFocused. To shift focus to another GameObject, use EventSystem.SetSelectedGameObject.
	 * A common use of this is allowing the user to type once focussed. Another way is outputting a message when the user clicks on a field(often seen when creating passwords).
	 */
	FORCEINLINE void OnFocus()
	{
		SelectAll();
	}

	/**
	 * Highlight the whole InputField.
	 *
	 * Sets the caretPosition to the length of the text and caretSelectPos to 0.
	 */
	FORCEINLINE void SelectAll()
	{
		SetCaretPositionInternal(Text.ToString().Len());
		SetCaretSelectPositionInternal(0);
	}

public:
	/**
	 * Move the caret index to end of text.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	void MoveTextEnd(bool bShiftDown);

	/**
	* Move the caret index to start of text.
	*/
	UFUNCTION(BlueprintCallable, Category = InputField)
	void MoveTextStart(bool bShiftDown);

private:
	int32 GetUnclampedCharacterLineFromPosition(const FVector2D& Pos, FTextGenerator& Generator) const;
	
	/**
	 * Given an input position in local space on the Text return the index for the selection cursor at this position.
	 */
	int32 GetCharacterIndexFromPosition(const FVector2D& Pos) const;
	bool MayDrag(UPointerEventData* EventData) const;
	
public:
	virtual void OnUpdateSelected(UBaseEventData* EventData) override;
	virtual void OnBeginDrag(UPointerEventData* EventData) override;
	virtual void OnDrag(UPointerEventData* EventData) override;
	virtual void OnEndDrag(UPointerEventData* EventData) override;
	virtual void OnPointerDown(UPointerEventData* EventData) override;

protected:
	enum class EEditState : uint8
	{
		Continue,
		Finish
	};

	/**
	* Process the Event and perform the appropriate action for that key.
	*/
	EEditState KeyPressed(const FKeyCharacterEvent& KeyCharacterEvent);
	
private:
	FString GetSelectedString(const FString& TextString) const;
	
	int32 FindNextWordBegin() const;
	void MoveRight(bool bShiftDown, bool bCtrlDown);
	
	int32 FindPrevWordBegin() const;
	void MoveLeft(bool bShiftDown, bool bCtrlDown);
	
	static int32 DetermineCharacterLine(int32 CharPos, const TSharedPtr<FTextGenerator>& Generator);

	/**
	 * Use cachedInputTextGenerator as the y component for the UICharInfo is not required
	 */
	int32 LineUpCharacterPosition(int32 OriginalPos, bool bGoToFirstChar);

	/**
	 * Use cachedInputTextGenerator as the y component for the UICharInfo is not required
	 */
	int32 LineDownCharacterPosition(int32 OriginalPos, bool bGoToLastChar);
	
	void MoveDown(bool bShiftDown);
	void MoveDown(bool bShiftDown, bool bGoToFirstChar);
	
	void MoveUp(bool bShiftDown);
	void MoveUp(bool bShiftDown, bool bGoToFirstChar);

	void Delete(FString& TextString);
	void ForwardSpace();
	void Backspace();

	/**
	 * Insert the character and update the label.
	 */
	void Insert(FString& TextString, TCHAR Character);
	
	void UpdateTouchKeyboardFromEditChanges();
	void SendOnValueChangedAndUpdateLabel();
	void SendOnValueChanged() const;
	
protected:
	/**
	 * Convenience function to make functionality to send the ::ref::SubmitEvent easier.
	 */
	void SendOnSubmit() const;

	/**
	 * Append the specified text to the end of the current text string. Appends character by character testing validation criteria.
	 */
	virtual void Append(const FString& TextString);
	
	/**
	 * Append a character to the input field, taking into account the validation of each character.
	 */
	virtual void Append(TCHAR Character);

	/**
	 * Update the Text associated with this input field.
	 */
	void UpdateLabel();

private:
	static int32 GetLineStartPosition(const TSharedPtr<FTextGenerator>& Gen, int32 Line);
	static int32 GetLineEndPosition(const TSharedPtr<FTextGenerator>& Gen, int32 Line);
	
	void SetDrawRangeToContainCaretPosition(int32 CaretPos);
	
	FORCEINLINE void MarkGeometryAsDirty()
	{
		FCanvasUpdateRegistry::RegisterCanvasElementForGraphicRebuild(this);
	}

public:
	//~ Begin ICanvasElementInterface Interface
	virtual const USceneComponent* GetTransform() const override { return this; };
	virtual void Rebuild(ECanvasUpdate Executing) override;
	virtual void LayoutComplete() override {};
	virtual void GraphicUpdateComplete() override {};
	virtual bool IsDestroyed() override { return !IsValid(this); };
	virtual FString ToString() override { return GetFName().ToString(); };
	//~ End ICanvasElementInterface Interface

private:
	void UpdateGeometry();
	void AssignPositioningIfNeeded() const;

	void OnFillVBO(FVertexHelper& VertexHelper);
	void GenerateCaret(IGraphicElementInterface* TextGraphicElem, FVertexHelper& VertexHelper, FVector2D RoundingOffset);

	void GenerateHighlight(IGraphicElementInterface* TextGraphicElem, FVertexHelper& VertexHelper, FVector2D RoundingOffset) const;
	
protected:
	/**
	 * Predefined validation functionality for different characterValidation types.
	 */
	TCHAR Validate(const FString& TextString, int32 Pos, TCHAR Character) const;

public:
	/**
	 * Function to activate the InputField to begin processing Events.
	 *
	 * Will only activate if deactivated.
	 */
	UFUNCTION(BlueprintCallable, Category = InputField)
	void ActivateInputField();

private:
	void ActivateInputFieldInternal();

public:
	/**
	 * What to do when the event system sends a submit Event.
	 */
	virtual void OnSelect(UBaseEventData* EventData) override;

	/**
	 * What to do when the event system sends a pointer click Event
	 */
	virtual void OnPointerClick(UPointerEventData* EventData) override;

public:
	/**
	* Function to deactivate the InputField to stop the processing of Events and send OnSubmit if not canceled.
	*/
	UFUNCTION(BlueprintCallable, Category = InputField)
	void DeactivateInputField();

public:
	/**
	 * What to do when the event system sends a Deselect Event. Defaults to deactivating the inputfield.
	 */
	virtual void OnDeselect(UBaseEventData* EventData) override;

	virtual void OnSubmit(UBaseEventData* EventData) override;

private:
	void EnforceContentType();
	void EnforceTextHOverflow() const;
	
	void SetToCustomIfContentTypeIsNot(TArray<EInputFieldContentType, TInlineAllocator<8>> AllowedContentType);
	void SetToCustom();

protected:
	virtual void DoStateTransition(ESelectableSelectionState InState, bool bInstant) override;

public:
	//~ Begin ILayoutElementInterface Interface
	virtual void CalculateLayoutInputHorizontal() override {};
	virtual void CalculateLayoutInputVertical() override {};
	virtual float GetMinWidth() override { return 0; };
	virtual float GetPreferredWidth() override;
	virtual float GetFlexibleWidth() override { return -1; };
	virtual float GetMinHeight() override { return 0; };
	virtual float GetPreferredHeight() override;
	virtual float GetFlexibleHeight() override { return -1; };
	virtual int32 GetLayoutPriority() override { return 1; };
	//~ End ILayoutElementInterface Interface

private:
	/** Virtual keyboard handler for an editable text layout */
	friend class FVirtualKeyboardEntry;
	class FVirtualKeyboardEntry : public IVirtualKeyboardEntry
	{
	public:
		static TSharedRef<FVirtualKeyboardEntry> Create(UInputFieldComponent* InInputField);

		virtual void SetTextFromVirtualKeyboard(const FText& InNewText, ETextEntryType TextEntryType) override;
		virtual void SetSelectionFromVirtualKeyboard(int InSelStart, int InSelEnd) override;
		virtual bool GetSelection(int& OutSelStart, int& OutSelEnd) override;

		virtual FText GetText() const override;
		virtual FText GetHintText() const override;
		virtual EKeyboardType GetVirtualKeyboardType() const override;
		virtual FVirtualKeyboardOptions GetVirtualKeyboardOptions() const override;
		virtual bool IsMultilineEntry() const override;

	private:
		FVirtualKeyboardEntry(UInputFieldComponent* InInputField);
		TWeakObjectPtr<UInputFieldComponent> InputField;;
	};

protected:
	/** Virtual keyboard handler for this text layout */
	TSharedPtr<FVirtualKeyboardEntry> VirtualKeyboardEntry;

private:
	friend class FTextInputMethodContext;
	
	class FTextInputMethodContext : public ITextInputMethodContext
	{
	public:
		static TSharedRef<FTextInputMethodContext> Create(UInputFieldComponent* InInputField);

		void KillContext();

		virtual bool IsComposing() override;
		virtual bool IsReadOnly() override;
		virtual uint32 GetTextLength() override;
		virtual void GetSelectionRange(uint32& BeginIndex, uint32& Length, ECaretPosition& InCaretPosition) override;
		virtual void SetSelectionRange(const uint32 BeginIndex, const uint32 Length, const ECaretPosition InCaretPosition) override;
		virtual void GetTextInRange(const uint32 BeginIndex, const uint32 Length, FString& OutString) override;
		virtual void SetTextInRange(const uint32 BeginIndex, const uint32 Length, const FString& InString) override;
		virtual int32 GetCharacterIndexFromPoint(const FVector2D& Point) override;
		virtual bool GetTextBounds(const uint32 BeginIndex, const uint32 Length, FVector2D& Position, FVector2D& Size) override;
		virtual void GetScreenBounds(FVector2D& Position, FVector2D& Size) override;
		virtual TSharedPtr<FGenericWindow> GetWindow() override;
		virtual void BeginComposition() override;
		virtual void UpdateCompositionRange(const int32 InBeginIndex, const uint32 InLength) override;
		virtual void EndComposition() override;

	private:
		FTextInputMethodContext(UInputFieldComponent* InInputField);

		TWeakObjectPtr<UInputFieldComponent> InputField;
		
		bool bIsComposing = false;
		int32 CompositionBeginIndex = 0;
		uint32 CompositionLength = 0;
		TSharedPtr<SBox> CachedWindow;
	};

protected:
	/** IME context for this input field */
	TSharedPtr<FTextInputMethodContext> TextInputMethodContext;
	
};
