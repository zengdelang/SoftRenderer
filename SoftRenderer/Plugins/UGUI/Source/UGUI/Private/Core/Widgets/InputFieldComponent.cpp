#include "Core/Widgets/InputFieldComponent.h"
#include "Core/Layout/RectTransformUtility.h"
#include "Core/Render/CanvasManager.h"
#include "Core/Widgets/InputFieldCaretComponent.h"
#include "EventSystem/EventData/PointerEventData.h"

/////////////////////////////////////////////////////
// UInputFieldComponent

constexpr int32 MaxTextLength = UINT16_MAX / 4 - 1;
const FString EmailSpecialCharacters = TEXT("!#$%&'*+-/=?^_`{|}~");
TCHAR Separators[] = { TEXT(' '), TEXT('.'), TEXT(','), TEXT('\t'), TEXT('\r'), TEXT('\n') };

FORCEINLINE int32 IndexOfAny(const FString& TextString, TCHAR AnyOf[], int32 AnyOfLength, int32 StartIndex)
{
	for (int32 Index = 0; Index < AnyOfLength; ++Index)
	{
		const TCHAR Char = AnyOf[Index];
		for (int32 TextIndex = StartIndex, Len = TextString.Len(); TextIndex < Len; ++TextIndex)
		{
			if (Char == TextString[TextIndex])
			{
				return TextIndex;
			}
		}
	}
	return -1;
}

UInputFieldComponent::UInputFieldComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Placeholder = nullptr;
	
	PlaceholderPath.Emplace(0);
	TextComponentPath.Emplace(1);

	ContentType = EInputFieldContentType::Standard;
	InputType = EInputFieldInputType::Standard;
	AsteriskChar = TEXT('*');
	KeyboardType = ETouchScreenKeyboardType::Keyboard_Default; 

	LineType = EInputFieldLineType::SingleLine;
	CharacterValidation = EInputFieldCharacterValidation::None;
	CharacterLimit = 0;

	CaretColor = FLinearColor(50.0f / 255, 50.0f / 255, 50.0f / 255, 1);
	SelectionColor = FLinearColor(168.0f / 255, 206.0f / 255, 255.0f / 255, 192.0f / 255);

	Text = FText::GetEmpty();
	OriginalText = FText::GetEmpty();

	CaretBlinkRate = 0.85;
	CaretWidth = 1;

	bReadOnly = false;
	bCustomCaretColor = false;
	bHideMobileInput = false;

	bPreventFontCallback = false;
	bAllowInput = false;
	bShouldActivateNextUpdate = false;
	bUpdateDrag = false;
	bDragPositionOutOfBounds = false;
	bCaretVisible = false;
	bNeedSetTimer = false;
	bWasCanceled = false;
	bHasDoneFocusTransition = false;
	bTouchKeyboardAllowsInPlaceEditing = false;
	bHasRegisteredTextInputMethodContext = false;
	bUseIntegratedKeyboard = false;

	CaretPosition = 0;
	CaretSelectPosition = 0;

	DrawStart = 0;
	DrawEnd = 0;

	BlinkStartTime = 0;
}

#if WITH_EDITORONLY_DATA

void UInputFieldComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UpdateEditChange();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UInputFieldComponent::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	UpdateEditChange();
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void UInputFieldComponent::UpdateEditChange()
{
	EnforceContentType();
	EnforceTextHOverflow();

	//This can be invoked before OnEnabled is called. So we shouldn't be accessing other objects, before OnEnable is called.
	if (!IsActiveAndEnabled())
		return;

	ClampPos( CaretPosition);
	ClampPos(CaretSelectPosition);

	UpdateLabel();
	if (bAllowInput)
		SetCaretActive();
}

#endif

void UInputFieldComponent::Awake()
{
	Super::Awake();

	TextComponent = FindChildBehaviourComponent(TextComponentPath);
	if (TextComponent)
	{
		TextComponent->SetGenerateCharacterAndLineInfo(true);
	}
	
	Placeholder = Cast<UBehaviourComponent>(FindChildBehaviourComponent(PlaceholderPath));

	EnforceTextHOverflow();
	
#if PLATFORM_IOS
	bool bUseIntegratedKeyboardTmp = false;
	GConfig->GetBool(TEXT("/Script/IOSRuntimeSettings.IOSRuntimeSettings"), TEXT("bUseIntegratedKeyboard"), bUseIntegratedKeyboardTmp, GEngineIni);
	bUseIntegratedKeyboard = bUseIntegratedKeyboardTmp;
#endif

	TextInputMethodContext = FTextInputMethodContext::Create(this);
	VirtualKeyboardEntry = FVirtualKeyboardEntry::Create(this);
}

void UInputFieldComponent::OnEnable()
{
	Super::OnEnable();
	
	DrawStart = 0;
	DrawEnd = Text.ToString().Len();

	// If we have a cached renderer then we had OnDisable called so just restore the material.
	// TODO 主要用于模板缓存裁剪
	//if (m_CachedInputRenderer != null)
	//	m_CachedInputRenderer.SetMaterial(m_TextComponent.GetModifiedMaterial(Graphic.defaultGraphicMaterial), Texture2D.whiteTexture);

	if (TextComponent)
	{
		auto GraphicElem = Cast<IGraphicElementInterface>(TextComponent.GetObject());
		if (GraphicElem)
		{
			MarkGeometryAsDirtyHandle = GraphicElem->OnDirtyVertsCallback.AddUObject(this, &UInputFieldComponent::MarkGeometryAsDirty);
			UpdateLabelHandle = GraphicElem->OnDirtyVertsCallback.AddUObject(this, &UInputFieldComponent::UpdateLabel);
			//TextComponent->RegisterDirtyMaterialCallback(UpdateCaretMaterial);  // TODO
		}
		UpdateLabel();		
	}

	FCanvasManager::AddLateUpdateObject(this);
}

void UInputFieldComponent::OnDisable()
{
	bNeedSetTimer = false;
	
	if (BlinkTimerHandle.IsValid())
	{
		const auto World = GetWorld();
		if (IsValid(World))
		{
			World->GetTimerManager().ClearTimer(BlinkTimerHandle);
		}
	}

	DeactivateInputField();
	if (TextComponent)
	{
		auto GraphicElem = Cast<IGraphicElementInterface>(TextComponent.GetObject());
		if (GraphicElem)
		{
			GraphicElem->OnDirtyVertsCallback.Remove(MarkGeometryAsDirtyHandle);
			GraphicElem->OnDirtyVertsCallback.Remove(UpdateLabelHandle);
			//m_TextComponent.UnregisterDirtyMaterialCallback(UpdateCaretMaterial); TODO
		}
	}
	FCanvasUpdateRegistry::UnRegisterCanvasElementForRebuild(this);

	// Clear needs to be called otherwise sync never happens as the object is disabled.
	if (CachedInputRenderer.IsValid())
		CachedInputRenderer->Clear();

	FCanvasManager::RemoveLateUpdateObject(this);

	Super::OnDisable();
}

void UInputFieldComponent::OnDestroy()
{
	if (bHasRegisteredTextInputMethodContext)
	{
		bHasRegisteredTextInputMethodContext = false;
		
		ITextInputMethodSystem* const TextInputMethodSystem = FSlateApplication::IsInitialized() ? FSlateApplication::Get().GetTextInputMethodSystem() : nullptr;
		if (TextInputMethodSystem)
		{
			const TSharedRef<FTextInputMethodContext> TextInputMethodContextRef = TextInputMethodContext.ToSharedRef();

			if (TextInputMethodSystem->IsActiveContext(TextInputMethodContextRef))
			{
				TextInputMethodSystem->DeactivateContext(TextInputMethodContextRef);
			}

			TextInputMethodSystem->UnregisterContext(TextInputMethodContextRef);
		}
	}
	
	if (TextInputMethodContext.IsValid())
	{
		TextInputMethodContext->KillContext();
	}

	if(FSlateApplication::IsInitialized() && FPlatformApplicationMisc::RequiresVirtualKeyboard())
	{
		// TODO 要不要手动调用下VirtualKeyboard的函数呢
		FSlateApplication::Get().ShowVirtualKeyboard(false, 0);
	}
	
	Super::OnDestroy();
}

void UInputFieldComponent::LateUpdate()
{
	// Update the text based on input.
	// TODO : Make LateUpdate a coroutine instead. Allows us to control the update to only be when the field is active.

	// Only activate if we are not already activated.
	if (bShouldActivateNextUpdate)
	{
		if (!IsFocused())
		{
			ActivateInputFieldInternal();
			bShouldActivateNextUpdate = false;
			return;
		}

#if !PLATFORM_WINDOWS
		if (FPlatformApplicationMisc::RequiresVirtualKeyboard())
		{
			if (IsFocused() && !bUseIntegratedKeyboard)
			{
				DeactivateInputField();
			}
		}
#endif

		// Reset as we are already activated.
		bShouldActivateNextUpdate = false;
	}

	AssignPositioningIfNeeded();

	if (!IsFocused() || InPlaceEditing())
		return;

	// TODO
}

void UInputFieldComponent::SetText(FString Value, bool bSendCallback)
{
	FString TextString = Text.ToString();
	if (TextString == Value)
		return;

	Value.ReplaceInline(TEXT("\0"), TEXT("")); // remove embedded nulls
	if (LineType == EInputFieldLineType::SingleLine)
	{
		Value.ReplaceInline(TEXT("\n"), TEXT(""));
		Value.ReplaceInline(TEXT("\t"), TEXT(""));
	}

	// If we have an input validator, validate the input and apply the character limit at the same time.
	if (OnValidateInput.IsBound() || CharacterValidation != EInputFieldCharacterValidation::None)
	{
		TextString = "";

		CaretPosition = CaretSelectPosition = Value.Len();
		const int32 CharactersToCheck = CharacterLimit > 0 ? FMath::Min(CharacterLimit, Value.Len()) : Value.Len();
		for (int32 Index = 0; Index < CharactersToCheck; ++Index)
		{
			const int32 Char = OnValidateInput.Execute(TextString, TextString.Len(), Value[Index]);
			if (Char != 0)
			{
				TextString.AppendChar(Char);
			}
		}
	}
	else
	{
		TextString = CharacterLimit > 0 && Value.Len() > CharacterLimit
			? Value.Mid(0, CharacterLimit)
			: Value;
	}

	Text = FText::FromString(TextString);
	
	if (CaretPosition > TextString.Len())
		CaretPosition = CaretSelectPosition = TextString.Len();
	else if (CaretSelectPosition > TextString.Len())
		CaretSelectPosition = TextString.Len();

	if (bSendCallback)
		SendOnValueChanged();
	UpdateLabel();
}

void UInputFieldComponent::CaretBlink()
{
	while (IsFocused() && CaretBlinkRate > 0)
	{
		// the blink rate is expressed as a frequency
		const float BlinkPeriod = 1.0 / CaretBlinkRate;

		// the caret should be ON if we are in the first half of the blink period
		const float DeltaTime = FApp::GetCurrentTime() - BlinkStartTime;
		const bool BlinkState = (DeltaTime - static_cast<int32>(DeltaTime)) < BlinkPeriod * 0.5;
		if (bCaretVisible != BlinkState)
		{
			bCaretVisible = BlinkState;
			if (!HasSelection())
				MarkGeometryAsDirty();
		}

		if (bNeedSetTimer)
		{
			const auto World = GetWorld();
			if (IsValid(World))
			{
				World->GetTimerManager().SetTimer(BlinkTimerHandle, FTimerDelegate::CreateUObject(this, &UInputFieldComponent::CaretBlink), 0.001f, false);
			}
		}
		
		// Then wait again.
		return;
	}

	bNeedSetTimer = false;
	if (BlinkTimerHandle.IsValid())
	{
		const auto World = GetWorld();
		if (IsValid(World))
		{
			World->GetTimerManager().ClearTimer(BlinkTimerHandle);
		}
	}
}

void UInputFieldComponent::SetCaretVisible()
{
	if (!bAllowInput)
		return;

	bCaretVisible = true;
	BlinkStartTime = FApp::GetCurrentTime();
	SetCaretActive();
}

void UInputFieldComponent::SetCaretActive()
{
	if (!bAllowInput)
		return;

	if (CaretBlinkRate > 0.0f)
	{
		if (!BlinkTimerHandle.IsValid())
		{
			const auto World = GetWorld();
			if (IsValid(World))
			{
				// Always ensure caret is initially visible since it can otherwise be confusing for a moment.
				bCaretVisible = true;
				bNeedSetTimer = true;
				World->GetTimerManager().SetTimer(BlinkTimerHandle, FTimerDelegate::CreateUObject(this, &UInputFieldComponent::CaretBlink), 0.001f, false);
			}	
		}
	}
	else
	{
		bCaretVisible = true;
	}
}

void UInputFieldComponent::MoveTextEnd(bool bShiftDown)
{
	const int32 Position = Text.ToString().Len();
	
	if (bShiftDown)
	{
		SetCaretSelectPositionInternal(Position);
	}
	else
	{
		SetCaretPositionInternal(Position);
		SetCaretSelectPositionInternal(GetCaretPositionInternal());
	}
	
	UpdateLabel();
}

void UInputFieldComponent::MoveTextStart(bool bShiftDown)
{
	const int32 Position = 0;
	
	if (bShiftDown)
	{
		SetCaretSelectPositionInternal(Position);
	}
	else
	{
		SetCaretPositionInternal(Position);
		SetCaretSelectPositionInternal(GetCaretPositionInternal());
	}

	UpdateLabel();
}

int32 UInputFieldComponent::GetUnclampedCharacterLineFromPosition(const FVector2D& Pos, FTextGenerator& Generator) const
{
	if (!IsMultiLine())
		return 0;

	// transform y to local scale
	const float Y = Pos.Y;
	float LastBottomY = 0.0f;

	for (int32 Index = 0, Count = Generator.Lines.Num(); Index < Count; ++Index)
	{
		const float TopY = Generator.Lines[Index].TopY;
		const float BottomY = TopY - Generator.Lines[Index].Height;

		// pos is somewhere in the leading above this line
		if (Y > TopY)
		{
			// determine which line we're closer to
			const float Leading = TopY - LastBottomY;
			if (Y > TopY - 0.5f * Leading)
				return Index - 1;
			else
				return Index;
		}

		if (Y > BottomY)
			return Index;

		LastBottomY = BottomY;
	}

	// Position is after last line.
	return Generator.Lines.Num();
}

int32 UInputFieldComponent::GetCharacterIndexFromPosition(const FVector2D& Pos) const
{
	auto& Gen = TextComponent->GetCachedTextGenerator();

	if (Gen.Lines.Num() == 0)
		return 0;

	const int32 Line = GetUnclampedCharacterLineFromPosition(Pos, Gen);
	if (Line < 0)
		return 0;
	if (Line >= Gen.Lines.Num())
		return Gen.CharacterCountVisible();

	const int32 StartCharIndex = Gen.Lines[Line].StartCharIdx;
	const int32 EndCharIndex = GetLineEndPosition(TextComponent->GetRawCachedTextGenerator(), Line);

	for (int32 Index = StartCharIndex; Index < EndCharIndex; ++Index)
	{
		if (Index >= Gen.CharacterCountVisible())
			break;

		FUICharInfo& CharInfo = Gen.Characters[Index];
		const FVector2D CharPos = CharInfo.CursorPos;

		const float DistToCharStart = Pos.X - CharPos.X;
		const float DistToCharEnd = CharPos.X + CharInfo.CharWidth - Pos.X;
		if (DistToCharStart < DistToCharEnd)
			return Index;
	}

	return EndCharIndex;
}

bool UInputFieldComponent::MayDrag(UPointerEventData* EventData) const
{
	return IsActiveAndEnabled() &&
			IsInteractableInHierarchy() &&
			EventData->Button == EPointerInputButton::InputButton_Left &&
			TextComponent &&
			(InPlaceEditing() || bHideMobileInput);
}

void UInputFieldComponent::OnUpdateSelected(UBaseEventData* EventData)
{
	if (!IsFocused())
		return;

	bool bConsumedEvent = false;

	const auto Input = GetInput();
	if (Input)
	{
		FKeyCharacterEvent KeyCharacterEvent;
		while (Input->PopKeyCharacterEvent(KeyCharacterEvent))
		{
			bConsumedEvent = true;	
			const auto ShouldContinue = KeyPressed(KeyCharacterEvent);
			if (ShouldContinue == EEditState::Finish)
			{
				DeactivateInputField();
				break;
			}
		}
	}

	if (bConsumedEvent)
		UpdateLabel();

	EventData->Use();
}

void UInputFieldComponent::OnBeginDrag(UPointerEventData* EventData)
{
	if (!MayDrag(EventData))
		return;

	bUpdateDrag = true;
}

void UInputFieldComponent::OnDrag(UPointerEventData* EventData)
{
	if (!MayDrag(EventData) || !TextComponent)
		return;

	const auto TextRectTransform = Cast<IGraphicElementInterface>(TextComponent.GetObject())->GetTransformComponent();
	
	FVector2D LocalMousePos;
	FRectTransformUtility::ScreenPointToLocalPointInRectangle(TextRectTransform, OwnerCanvas, FVector2D(EventData->Position), LocalMousePos);
	SetCaretSelectPositionInternal(GetCharacterIndexFromPosition(LocalMousePos) + DrawStart);

	MarkGeometryAsDirty();

	FVector LocalPos;
	bDragPositionOutOfBounds = !FRectTransformUtility::RectangleContainsScreenPoint(TextRectTransform, OwnerCanvas,
		FVector2D(EventData->Position), LocalPos);
	
	if (bDragPositionOutOfBounds && bUpdateDrag)
	{
		const auto& TextRect = TextRectTransform->GetRect();
		if (IsMultiLine())
		{
			if (LocalMousePos.Y > TextRect.GetYMax())
				MoveUp(true, true);
			else if (LocalMousePos.Y < TextRect.YMin)
				MoveDown(true, true);
		}
		else
		{
			if (LocalMousePos.X < TextRect.XMin)
				MoveLeft(true, false);
			else if (LocalMousePos.X > TextRect.GetXMax())
				MoveRight(true, false);
		}
		UpdateLabel();
	}

	EventData->Use();
}

void UInputFieldComponent::OnEndDrag(UPointerEventData* EventData)
{
	if (!MayDrag(EventData))
		return;

	bUpdateDrag = false;
}

void UInputFieldComponent::OnPointerDown(UPointerEventData* EventData)
{
	if (bHasRegisteredTextInputMethodContext)
	{
		ITextInputMethodSystem* const TextInputMethodSystem = FSlateApplication::IsInitialized() ? FSlateApplication::Get().GetTextInputMethodSystem() : nullptr;
		if (TextInputMethodSystem)
		{
			const TSharedRef<FTextInputMethodContext> TextInputMethodContextRef = TextInputMethodContext.ToSharedRef();
			if (TextInputMethodSystem->IsActiveContext(TextInputMethodContextRef))
			{
				TextInputMethodSystem->DeactivateContext(TextInputMethodContextRef);
			}
			TextInputMethodSystem->ActivateContext(TextInputMethodContext.ToSharedRef());
		}
	}

	if (!MayDrag(EventData) || !TextComponent)
		return;

	const auto World = GetWorld();
	const auto EventSystem = UEventSystemComponent::GetCurrentEventSystem(World);
	if (IsValid(EventSystem))
	{
		EventSystem->SetSelectedGameObject(this, EventData);
	}

	const bool bHadFocusBefore = bAllowInput;
	Super::OnPointerDown(EventData);

	if (!InPlaceEditing())
	{
		// TODO
		/*if (m_Keyboard == null || !m_Keyboard.active)
		{
			OnSelect(eventData);
			return;
		}*/
	}

	// Only set caret position if we didn't just get focus now.
	// Otherwise it will overwrite the select all on focus.
	if (bHadFocusBefore)
	{		
		FVector2D LocalMousePos;	
		FRectTransformUtility::ScreenPointToLocalPointInRectangle(Cast<IGraphicElementInterface>(TextComponent.GetObject())->GetTransformComponent(),
			OwnerCanvas, FVector2D(EventData->Position), LocalMousePos);
		const auto NewPos = GetCharacterIndexFromPosition(LocalMousePos) + DrawStart;
		SetCaretPositionInternal(NewPos);
		SetCaretSelectPositionInternal(NewPos);
	}

	UpdateLabel();
	EventData->Use();
}

UInputFieldComponent::EEditState UInputFieldComponent::KeyPressed(const FKeyCharacterEvent& KeyCharacterEvent)
{
	const bool bControlDown = KeyCharacterEvent.IsControlDown();
	const bool bShiftDown = KeyCharacterEvent.IsShiftDown();
	const bool bAltDown = KeyCharacterEvent.IsAltDown();
	const bool bCtrlOnly = bControlDown && !bAltDown && !bShiftDown;
	
	const FKey Key = KeyCharacterEvent.GetKey();
	if (Key == EKeys::BackSpace)
	{
		Backspace();
		return EEditState::Continue;
	}
	else if (Key == EKeys::Delete)
	{
		ForwardSpace();
		return EEditState::Continue;
	}
	else if (Key == EKeys::Home)
	{
		MoveTextStart(bShiftDown);
		return EEditState::Continue;
	}
	else if (Key == EKeys::End)
	{
		MoveTextEnd(bShiftDown);
		return EEditState::Continue;
	}
	else if (Key == EKeys::Left)
	{
		MoveLeft(bShiftDown, bControlDown);
		return EEditState::Continue;
	}
	else if (Key == EKeys::Right)
	{
		MoveRight(bShiftDown, bControlDown);
		return EEditState::Continue;
	}
	else if (Key == EKeys::Up)
	{
		MoveUp(bShiftDown);
		return EEditState::Continue;
	}
	else if (Key == EKeys::Down)
	{
		MoveDown(bShiftDown);
		return EEditState::Continue;
	}
	else if (Key == EKeys::A)
	{
		// Select All
		if (bCtrlOnly)
		{
			SelectAll();
			return EEditState::Continue;
		}
	}
	else if (Key == EKeys::C)
	{
		// Copy
		if (bCtrlOnly)
		{
			const FString TextString = Text.ToString();
			if (InputType != EInputFieldInputType::Password)
			{
				// Copy text to clipboard
				FPlatformApplicationMisc::ClipboardCopy(*GetSelectedString(TextString));
			}
			else
			{
				FPlatformApplicationMisc::ClipboardCopy(TEXT(""));
			}
			return EEditState::Continue;
		}
	}
	else if (Key == EKeys::V)
	{
		// Paste
		if (bCtrlOnly)
		{
			FString ClipboardContent;
			FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);
			Append(ClipboardContent);
			UpdateLabel();
			return EEditState::Continue;
		}
	}
	else if (Key == EKeys::X)
	{
		// Cut
		if (bCtrlOnly)
		{
			FString TextString = Text.ToString();
			if (InputType != EInputFieldInputType::Password)
			{
				// Copy text to clipboard
				FPlatformApplicationMisc::ClipboardCopy(*GetSelectedString(TextString));
			}
			else
			{
				FPlatformApplicationMisc::ClipboardCopy(TEXT(""));
			}

			Delete(TextString);
			UpdateTouchKeyboardFromEditChanges();
			SendOnValueChangedAndUpdateLabel();
			return EEditState::Continue;
		}
	}
	else if (Key == EKeys::Enter)
	{
		// Submit
		if (LineType != EInputFieldLineType::MultiLineNewline)
		{
			return EEditState::Finish;
		}
	}
	else if (Key == EKeys::Escape)
	{
		bWasCanceled = true;
		return EEditState::Finish;
	}

    TCHAR Character = KeyCharacterEvent.GetCharacter();

    // Don't allow return chars or tabulator key to be entered into single line fields.
    if (!IsMultiLine() && (Character == TEXT('\t') || Character == TEXT('\r') || Character == 10))
        return EEditState::Continue;

	switch (Character)
	{
		case 1:				// Swallow Ctrl+A, we handle that through OnKeyDown
		case 3:				// Swallow Ctrl+C, we handle that through OnKeyDown
		case 22:			// Swallow Ctrl+V, we handle that through OnKeyDown
		case 24:			// Swallow Ctrl+X, we handle that through OnKeyDown
		case 25:			// Swallow Ctrl+Y, we handle that through OnKeyDown
		case 26:			// Swallow Ctrl+Z, we handle that through OnKeyDown
		case 27:			// Swallow ESC, we handle that through OnKeyDown
		case 127:			// Swallow CTRL+Backspace, we handle that through OnKeyDown
			return EEditState::Continue;
		default: ;
	}

    // Convert carriage return and end-of-text characters to newline.
	if (Character == TEXT('\r'))
       Character = TEXT('\n');

	Append(Character);

    return EEditState::Continue;	
}

FString UInputFieldComponent::GetSelectedString(const FString& TextString) const
{
	if (!HasSelection())
		return TEXT("");

	int32 StartPos = GetCaretPositionInternal();
	int32 EndPos = GetCaretSelectPositionInternal();

	// Ensure startPos is always less then endPos to make the code simpler
	if (StartPos > EndPos)
	{
		const int32 Temp = StartPos;
		StartPos = EndPos;
		EndPos = Temp;
	}

	return TextString.Mid(StartPos, EndPos - StartPos);
}

int32 UInputFieldComponent::FindNextWordBegin() const
{
	const FString TextString = Text.ToString();
	if (GetCaretSelectPositionInternal() + 1 >= Text.ToString().Len())
		return TextString.Len();

	int32 SpaceLoc = IndexOfAny(TextString, Separators, 6,GetCaretSelectPositionInternal() + 1);

	if (SpaceLoc == -1)
		SpaceLoc = TextString.Len();
	else
		SpaceLoc++;

	return SpaceLoc;
}

void UInputFieldComponent::MoveRight(bool bShiftDown, bool bCtrlDown)
{
	if (HasSelection() && !bShiftDown)
	{
		// By convention, if we have a selection and move right without holding shift,
		// we just place the cursor at the end.
		const auto NewPos = FMath::Max(GetCaretPositionInternal(), GetCaretSelectPositionInternal());
		SetCaretPositionInternal(NewPos);
		SetCaretSelectPositionInternal(NewPos);
		return;
	}

	int32 Position;
	if (bCtrlDown)
		Position = FindNextWordBegin();
	else
		Position = GetCaretSelectPositionInternal() + 1;

	if (bShiftDown)
	{
		SetCaretSelectPositionInternal(Position);
	}	
	else
	{
		SetCaretPositionInternal(Position);
		SetCaretSelectPositionInternal(Position);
	}
}

int32 UInputFieldComponent::FindPrevWordBegin() const
{
	if (GetCaretSelectPositionInternal() - 2 < 0)
		return 0;

	int32 SpaceLoc = IndexOfAny(Text.ToString(), Separators, 6,GetCaretSelectPositionInternal() - 2);
	
	if (SpaceLoc == -1)
		SpaceLoc = 0;
	else
		SpaceLoc++;

	return SpaceLoc;
}

void UInputFieldComponent::MoveLeft(bool bShiftDown, bool bCtrlDown)
{
	if (HasSelection() && !bShiftDown)
	{
		// By convention, if we have a selection and move left without holding shift,
		// we just place the cursor at the start
		const auto NewPos = FMath::Min(GetCaretPositionInternal(), GetCaretSelectPositionInternal());
		SetCaretPositionInternal(NewPos);
		SetCaretSelectPositionInternal(NewPos);
		return;
	}

	int32 Position;
	if (bCtrlDown)
		Position = FindPrevWordBegin();
	else
		Position = GetCaretSelectPositionInternal() - 1;

	if (bShiftDown)
	{
		SetCaretSelectPositionInternal(Position);
	}	
	else
	{
		SetCaretPositionInternal(Position);
		SetCaretSelectPositionInternal(Position);
	}
}

int32 UInputFieldComponent::DetermineCharacterLine(int32 CharPos, const TSharedPtr<FTextGenerator>& Generator)
{
	for (int32 Index = 0, Count = Generator->Lines.Num() - 1; Index < Count; ++Index)
	{
		if (Generator->Lines[Index + 1].StartCharIdx > CharPos)
			return Index;
	}
	return Generator->Lines.Num() - 1;
}

int32 UInputFieldComponent::LineUpCharacterPosition(int32 OriginalPos, bool bGoToFirstChar)
{
	if (OriginalPos >= GetCachedInputTextGenerator()->Characters.Num())
		return 0;

	FUICharInfo& OriginChar = GetCachedInputTextGenerator()->Characters[OriginalPos];
	const int32 OriginLine = DetermineCharacterLine(OriginalPos, GetCachedInputTextGenerator());

	// We are on the first line return first character
	if (OriginLine <= 0)
		return bGoToFirstChar ? 0 : OriginalPos;

	const int32 EndCharIdx = GetCachedInputTextGenerator()->Lines[OriginLine].StartCharIdx - 1;

	for (int32 Index = GetCachedInputTextGenerator()->Lines[OriginLine - 1].StartCharIdx; Index < EndCharIdx; ++Index)
	{
		if (GetCachedInputTextGenerator()->Characters[Index].CursorPos.X >= OriginChar.CursorPos.X)
			return Index;
	}
	return EndCharIdx;
}

int32 UInputFieldComponent::LineDownCharacterPosition(int32 OriginalPos, bool bGoToLastChar)
{
	if (OriginalPos >= GetCachedInputTextGenerator()->CharacterCountVisible())
		return Text.ToString().Len();

	FUICharInfo& OriginChar = GetCachedInputTextGenerator()->Characters[OriginalPos];
	const int32 OriginLine = DetermineCharacterLine(OriginalPos, GetCachedInputTextGenerator());

	// We are on the last line return last character
	if (OriginLine + 1 >= GetCachedInputTextGenerator()->Lines.Num())
		return bGoToLastChar ? Text.ToString().Len() : OriginalPos;

	// Need to determine end line for next line.
	const int32 EndCharIdx = GetLineEndPosition(GetCachedInputTextGenerator(), OriginLine + 1);

	for (int32 Index = GetCachedInputTextGenerator()->Lines[OriginLine + 1].StartCharIdx; Index < EndCharIdx; ++Index)
	{
		if (GetCachedInputTextGenerator()->Characters[Index].CursorPos.X >= OriginChar.CursorPos.X)
			return Index;
	}
	return EndCharIdx;
}

void UInputFieldComponent::MoveDown(bool bShiftDown)
{
	MoveDown(bShiftDown, true);
}

void UInputFieldComponent::MoveDown(bool bShiftDown, bool bGoToFirstChar)
{
	if (HasSelection() && !bShiftDown)
	{
		// If we have a selection and press down without shift,
		// set caret position to end of selection before we move it down.
		const auto NewPos = FMath::Max(GetCaretPositionInternal(), GetCaretSelectPositionInternal());
		SetCaretPositionInternal(NewPos);
		SetCaretSelectPositionInternal(NewPos);
	}

	const int32 Position = IsMultiLine() ? LineDownCharacterPosition(GetCaretSelectPositionInternal(), bGoToFirstChar) : Text.ToString().Len();
	if (bShiftDown)
	{
		SetCaretSelectPositionInternal(Position);
	}	
	else
	{
		SetCaretPositionInternal(Position);
		SetCaretSelectPositionInternal(Position);
	}
}

void UInputFieldComponent::MoveUp(bool bShiftDown)
{
	MoveUp(bShiftDown, true);
}

void UInputFieldComponent::MoveUp(bool bShiftDown, bool bGoToFirstChar)
{
	if (HasSelection() && !bShiftDown)
	{
		// If we have a selection and press up without shift,
		// set caret position to start of selection before we move it up.
		const auto NewPos = FMath::Min(GetCaretPositionInternal(), GetCaretSelectPositionInternal());
		SetCaretPositionInternal(NewPos);
		SetCaretSelectPositionInternal(NewPos);
	}

	const int32 Position = IsMultiLine() ? LineUpCharacterPosition(GetCaretSelectPositionInternal(), bGoToFirstChar) : 0;
	if (bShiftDown)
	{
		SetCaretSelectPositionInternal(Position);
	}	
	else
	{
		SetCaretPositionInternal(Position);
		SetCaretSelectPositionInternal(Position);
	}
}

void UInputFieldComponent::Delete(FString& TextString)
{
	if (bReadOnly)
		return;

	if (GetCaretPositionInternal() == GetCaretSelectPositionInternal())
		return;

	if (GetCaretPositionInternal() < GetCaretSelectPositionInternal())
	{
		TextString = TextString.Mid(0, GetCaretPositionInternal()) + TextString.Mid(GetCaretSelectPositionInternal(), TextString.Len() - GetCaretSelectPositionInternal());
		Text = FText::FromString(TextString);
		SetCaretSelectPositionInternal(GetCaretPositionInternal());
	}
	else
	{
		TextString = TextString.Mid(0, GetCaretSelectPositionInternal()) + TextString.Mid(GetCaretPositionInternal(), TextString.Len() - GetCaretPositionInternal());
		Text = FText::FromString(TextString);
		SetCaretPositionInternal(GetCaretSelectPositionInternal());
	}
}

void UInputFieldComponent::ForwardSpace()
{
	if (bReadOnly)
		return;

	FString TextString = Text.ToString();
	if (HasSelection())
	{
		Delete(TextString);
		
		UpdateTouchKeyboardFromEditChanges();
		SendOnValueChangedAndUpdateLabel();
	}
	else
	{
		if (GetCaretPositionInternal() < TextString.Len())
		{
			TextString.RemoveAt(GetCaretPositionInternal(), 1, false);
			Text = FText::FromString(TextString);
			
			UpdateTouchKeyboardFromEditChanges();
			SendOnValueChangedAndUpdateLabel();
		}
	}
}

void UInputFieldComponent::Backspace()
{
	if (bReadOnly)
		return;

	FString TextString = Text.ToString();
	if (HasSelection())
	{
		Delete(TextString);

		UpdateTouchKeyboardFromEditChanges();
		SendOnValueChangedAndUpdateLabel();
	}
	else
	{
		if (GetCaretPositionInternal() > 0)
		{
			const int32 NewPos = GetCaretPositionInternal() - 1;
			TextString.RemoveAt(NewPos, 1, false);
			Text = FText::FromString(TextString);
			
			SetCaretPositionInternal(NewPos);
			SetCaretSelectPositionInternal(NewPos);

			UpdateTouchKeyboardFromEditChanges();
			SendOnValueChangedAndUpdateLabel();
		}
	}
}

void UInputFieldComponent::Insert(FString& TextString, TCHAR Character)
{
	if (bReadOnly)
		return;

	Delete(TextString);

	// Can't go past the character limit
	if (CharacterLimit > 0 && TextString.Len() >= CharacterLimit)
		return;
 
	TextString.InsertAt(CaretPosition, Character);
	Text = FText::FromString(TextString);

	const int32 NewCaretPos = GetCaretPositionInternal() + 1;
	SetCaretPositionInternal(NewCaretPos);
	SetCaretSelectPositionInternal(NewCaretPos);

	UpdateTouchKeyboardFromEditChanges();
	SendOnValueChanged();
}

void UInputFieldComponent::UpdateTouchKeyboardFromEditChanges()
{
	// TODO
}

void UInputFieldComponent::SendOnValueChangedAndUpdateLabel()
{
	SendOnValueChanged();
	UpdateLabel();
}

void UInputFieldComponent::SendOnValueChanged() const
{
	OnValueChanged.Broadcast(Text);
}

void UInputFieldComponent::SendOnSubmit() const
{
	OnEndEdit.Broadcast(Text);
}

void UInputFieldComponent::Append(const FString& TextString)
{
	if (bReadOnly)
		return;

	if (!InPlaceEditing())
		return;

	for (int32 Index = 0, Count = TextString.Len(); Index < Count; ++Index)
	{
		const auto Char = TextString[Index];

		if (Char >= TEXT(' ') || Char == TEXT('\t') || Char == TEXT('\r') || Char == 10 || Char == TEXT('\n'))
		{
			Append(Char);
		}
	}
}

void UInputFieldComponent::Append(TCHAR Character)
{
	if (!IsValidChar(Character))
		return;

	// If the input is invalid, skip it
	if (Character == 0)
		return;
	
	// We do not currently support surrogate pairs
	if (StringConv::IsHighSurrogate(Character) || StringConv::IsLowSurrogate(Character))
		return;

	FString TextString = Text.ToString();
	if (bReadOnly || TextString.Len() >= MaxTextLength)
		return;

	if (!InPlaceEditing())
		return;

	// If we have an input validator, validate the input first
	const int32 InsertionPoint = FMath::Min(GetSelectionFocusPosition(), GetSelectionAnchorPosition());
	if (OnValidateInput.IsBound())
	{
		Character = OnValidateInput.Execute(TextString, InsertionPoint, Character);
	}	
	else if (CharacterValidation != EInputFieldCharacterValidation::None)
	{
		Character = Validate(TextString, InsertionPoint, Character);
	}
	
	// Append the character and update the label
	Insert(TextString, Character);
}

void UInputFieldComponent::UpdateLabel()
{
	if (TextComponent && TextComponent->GetFont() != nullptr && !bPreventFontCallback)
	{
		bPreventFontCallback = true;

		FString FullText = Text.ToString();
		const auto World = GetWorld();
		const auto EventSystem = UEventSystemComponent::GetCurrentEventSystem(World);
		if (IsValid(EventSystem) && EventSystem->GetCurrentSelectedGameObject() == this && GetCompositionString().Len() > 0)
		{
			FullText = FullText.Mid(0, CaretPosition) + GetCompositionString() + FullText.Mid(CaretPosition);
		}

		FString Processed;
		if (InputType == EInputFieldInputType::Password)
		{
			for (int32 Index = 0, Count = FullText.Len(); Index < Count; ++Index)
			{
				Processed.AppendChar(AsteriskChar);
			}
		}
		else
		{
			Processed = FullText;
		}

		const bool bIsEmpty = FullText.IsEmpty();

		if (IsValid(Placeholder))
		{
			Placeholder->SetEnabled(bIsEmpty);
		}

		// If not currently editing the text, set the visible range to the whole text.
		// The UpdateLabel method will then truncate it to the part that fits inside the Text area.
		// We can't do this when text is being edited since it would discard the current scroll,
		// which is defined by means of the m_DrawStart and m_DrawEnd indices.
		if (!bAllowInput)
		{
			DrawStart = 0;
			DrawEnd = Text.ToString().Len();
		}

		if (!bIsEmpty)
		{
			// Determine what will actually fit into the given line
			const FVector2D Extents = TextComponent->GetTextTransformComponent()->GetRect().GetSize();
			
			FTextGenerationSettings Settings = TextComponent->GetGenerationSettings(Extents);
			if (IsMultiLine())
			{
				Settings.HorizontalOverflow = EHorizontalWrapMode::HorizontalWrapMode_Wrap;
			}
			else
			{
				Settings.HorizontalOverflow = EHorizontalWrapMode::HorizontalWrapMode_Overflow;
			}
			Settings.VerticalOverflow = EVerticalWrapMode::VerticalWrapMode_Overflow;
			Settings.bRichText = false;
			
			GetCachedInputTextGenerator()->Populate(Processed, Settings, this);
			
			SetDrawRangeToContainCaretPosition(GetCaretSelectPositionInternal());
			
			Processed = Processed.Mid(DrawStart, FMath::Min(DrawEnd, Processed.Len()) - DrawStart);
			
			SetCaretVisible();
		}

		TextComponent->SetSupportRichText(false);
		TextComponent->SetText(FText::FromString(Processed));
		MarkGeometryAsDirty();
		
		bPreventFontCallback = false;
	}
}

int32 UInputFieldComponent::GetLineStartPosition(const TSharedPtr<FTextGenerator>& Gen, int32 Line)
{
	Line = FMath::Clamp(Line, 0, Gen->Lines.Num() - 1);
	return Gen->Lines[Line].StartCharIdx;
}

int32 UInputFieldComponent::GetLineEndPosition(const TSharedPtr<FTextGenerator>& Gen, int32 Line)
{
	Line = FMath::Max(Line, 0);
	if (Line + 1 < Gen->Lines.Num())
		return Gen->Lines[Line + 1].StartCharIdx - 1;
	return Gen->CharacterCountVisible();
}

void UInputFieldComponent::SetDrawRangeToContainCaretPosition(int32 CaretPos)
{
	// We don't have any generated lines generation is not valid.
	if (GetCachedInputTextGenerator()->Lines.Num() <= 0)
		return;

	// the extents gets modified by the pixel density, so we need to use the generated extents since that will be in the same 'space' as
	// the values returned by the TextGenerator.lines[x].height for instance.
	const FVector2D Extents = InputTextCache->Extents;

	if (IsMultiLine())
	{
		auto& Lines = GetCachedInputTextGenerator()->Lines;
		const int32 CaretLine = DetermineCharacterLine(CaretPos, GetCachedInputTextGenerator());

		if (CaretPos > DrawEnd)
		{
			// Caret comes after drawEnd, so we need to move drawEnd to the end of the line with the caret
			DrawEnd = GetLineEndPosition(GetCachedInputTextGenerator(), CaretLine);
			const float BottomY = Lines[CaretLine].TopY - Lines[CaretLine].Height;
			if (CaretLine == Lines.Num() - 1)
			{
				// Remove interline spacing on last line.
				// BottomY += Lines[CaretLine].leading;  // TODO
			}
			
			int32 StartLine = CaretLine;
			while (StartLine > 0)
			{
				const float TopY = Lines[StartLine - 1].TopY;
				if (TopY - BottomY > Extents.Y)
					break;
				StartLine--;
			}
			DrawStart = GetLineStartPosition(GetCachedInputTextGenerator(), StartLine);
		}
		else
		{
			if (CaretPos < DrawStart)
			{
				// Caret comes before drawStart, so we need to move drawStart to an earlier line start that comes before caret.
				DrawStart = GetLineStartPosition(GetCachedInputTextGenerator(), CaretLine);
			}

			int32 StartLine = DetermineCharacterLine(DrawStart, GetCachedInputTextGenerator());
			int32 EndLine = StartLine;

			float TopY = Lines[StartLine].TopY;
			float BottomY = Lines[EndLine].TopY - Lines[EndLine].Height;

			if (EndLine == Lines.Num() - 1)
			{
				// Remove interline spacing on last line.
				// BottomY += Lines[EndLine].leading;  // TODO
			}

			while (EndLine < Lines.Num() - 1)
			{
				BottomY = Lines[EndLine + 1].TopY - Lines[EndLine + 1].Height;

				if (EndLine + 1 == Lines.Num() - 1)
				{
					// Remove interline spacing on last line.
					//BottomY += Lines[EndLine + 1].leading;  // TODO
				}

				if (TopY - BottomY > Extents.Y)
					break;
				++EndLine;
			}

			DrawEnd = GetLineEndPosition(GetCachedInputTextGenerator(), EndLine);

			while (StartLine > 0)
			{
				TopY = Lines[StartLine - 1].TopY;
				if (TopY - BottomY > Extents.Y)
					break;
				StartLine--;
			}
			DrawStart = GetLineStartPosition(GetCachedInputTextGenerator(), StartLine);
		}
	}
	else
	{
		auto& Characters = InputTextCache->Characters;
		const int32 CharacterCountVisible = InputTextCache->CharacterCountVisible();
		if (DrawEnd > CharacterCountVisible)
			DrawEnd = CharacterCountVisible;

		float Width = 0.0f;
		if (CaretPos > DrawEnd || (CaretPos == DrawEnd && DrawStart > 0))
		{
			// fit characters from the caretPos leftward
			DrawEnd = CaretPos;
			for (DrawStart = DrawEnd - 1; DrawStart >= 0; --DrawStart)
			{
				if (!Characters.IsValidIndex(DrawStart))
					continue;
				
				if (Width + Characters[DrawStart].CharWidth > Extents.X)
					break;

				Width += Characters[DrawStart].CharWidth;
			}
			++DrawStart; // move right one to the last character we could fit on the left
		}
		else
		{
			if (CaretPos < DrawStart)
				DrawStart = CaretPos;

			DrawEnd = DrawStart;
		}

		// fit characters rightward
		for (; DrawEnd < CharacterCountVisible; ++DrawEnd)
		{
			Width += Characters[DrawEnd].CharWidth;
			if (Width > Extents.X)
				break;
		}
	}
}

void UInputFieldComponent::Rebuild(ECanvasUpdate Executing)
{
	switch(Executing)
	{
		case ECanvasUpdate::CanvasUpdate_LatePreRender:
			UpdateGeometry();
			break;
		default: ;
	}
}

void UInputFieldComponent::UpdateGeometry()
{
	// No need to draw a cursor on mobile as its handled by the devices keyboard.
	if (!ShouldHideMobileInput())
		return;

	if (!CachedInputRenderer.IsValid() && TextComponent)
	{
		UInputFieldCaretComponent* CaretComponent = NewObject<UInputFieldCaretComponent>(GetOwner(), NAME_None, RF_Transient);
#if WITH_EDITOR
		CaretComponent->bIsEditorOnly = true;
		CaretComponent->CreationMethod = EComponentCreationMethod::Instance;
#endif
		CaretComponent->RegisterComponent();
		CaretComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		CaretComponent->SetRelativeTransform(FTransform::Identity);   // TODO 好多地方用了SetReleativeTransform 不应该这样搞的，没走RectTransform
		
		CaretComponent->AwakeFromLoad();

		CaretRectTrans = CaretComponent;
		CachedInputRenderer = CaretComponent->CanvasRenderer;

		if (IsValid(Placeholder))
		{
			Placeholder->bDisableTransformParentChanged = true;
			const auto PlaceholderParent = Placeholder->GetAttachParent();
			Placeholder->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
			Placeholder->bDisableTransformParentChanged = false;
						
			if (IsValid(PlaceholderParent))
			{
				Placeholder->AttachToComponent(PlaceholderParent, FAttachmentTransformRules::KeepRelativeTransform);
			}
		}

		IGraphicElementInterface* TextGraphicElem =  Cast<IGraphicElementInterface>(TextComponent.GetObject());
		if (TextGraphicElem)
		{
			auto TextSceneComp = TextGraphicElem->GetTransformComponent();
			if (IsValid(TextSceneComp))
			{
				TextSceneComp->bDisableTransformParentChanged = true;
				const auto TextParent = TextSceneComp->GetAttachParent();
				TextSceneComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
				TextSceneComp->bDisableTransformParentChanged = false;
						
				if (IsValid(TextParent))
				{
					TextSceneComp->AttachToComponent(TextParent, FAttachmentTransformRules::KeepRelativeTransform);
				}
			}
		}

		AssignPositioningIfNeeded();
	}

	if (!CachedInputRenderer.IsValid())
		return;

	IGraphicElementInterface::StaticVertexHelper.Reset();
	OnFillVBO(IGraphicElementInterface::StaticVertexHelper);
	CachedInputRenderer->FillMesh(IGraphicElementInterface::StaticVertexHelper);
}

void UInputFieldComponent::AssignPositioningIfNeeded() const
{
	IGraphicElementInterface* TextGraphicElem =  Cast<IGraphicElementInterface>(TextComponent.GetObject());
	if (CaretRectTrans.IsValid() && TextGraphicElem)
	{
		const auto TextSceneComp = TextGraphicElem->GetTransformComponent();
		if (IsValid(TextSceneComp) &&
			(TextSceneComp->GetPivot() != CaretRectTrans->GetPivot() ||
			 /*
			 caretRectTrans.localRotation != m_TextComponent.rectTransform.localRotation ||
			 caretRectTrans.localScale != m_TextComponent.rectTransform.localScale ||*/
			 TextSceneComp->GetLocalLocation() != CaretRectTrans->GetLocalLocation() ||
			 TextSceneComp->GetAnchorMin() != CaretRectTrans->GetAnchorMin() ||
			 TextSceneComp->GetAnchorMax() != CaretRectTrans->GetAnchorMax() ||
			 TextSceneComp->GetAnchoredPosition() != CaretRectTrans->GetAnchoredPosition() ||
			 TextSceneComp->GetSizeDelta() != CaretRectTrans->GetSizeDelta() 
				))
		{
			/*
			caretRectTrans.localRotation = m_TextComponent.rectTransform.localRotation;
			caretRectTrans.localScale = m_TextComponent.rectTransform.localScale;*/  // TODO
			CaretRectTrans->SetLocalLocation(TextSceneComp->GetLocalLocation());
			CaretRectTrans->SetAnchorAndSizeAndPosition(TextSceneComp->GetAnchorMin(), TextSceneComp->GetAnchorMax(), TextSceneComp->GetPivot(), TextSceneComp->GetAnchoredPosition());
			CaretRectTrans->SetSizeDelta(TextSceneComp->GetSizeDelta());
		}
	}
}

void UInputFieldComponent::OnFillVBO(FVertexHelper& VertexHelper)
{
	if (!IsFocused())
	{
		return;
	}

	IGraphicElementInterface* TextGraphicElem =  Cast<IGraphicElementInterface>(TextComponent.GetObject());
	if (!TextGraphicElem)
	{
		return;
	}
	
	const FVector2D RoundingOffset = TextGraphicElem->PixelAdjustPoint(FVector2D::ZeroVector);
	if (!HasSelection())
	{
		GenerateCaret(TextGraphicElem, VertexHelper, RoundingOffset);
	}
	else
	{
		GenerateHighlight(TextGraphicElem, VertexHelper, RoundingOffset);
	}
}

void UInputFieldComponent::GenerateCaret(IGraphicElementInterface* TextGraphicElem, FVertexHelper& VertexHelper, FVector2D RoundingOffset)
{
	if (!bCaretVisible)
		return;

	if (CursorVerts.Num() == 0)
	{
		for (int32 Index = 0; Index < 4; ++Index)
		{
			CursorVerts.Add(FUIVertex::SimpleVertex);
			CursorVerts[Index].UV0 = FVector2D::ZeroVector;
		}
	}

	const float Width = CaretWidth;
	const int32 AdjustedPos = FMath::Max(0, GetCaretPositionInternal() - DrawStart);

	auto Gen = TextComponent->GetRawCachedTextGenerator();
	if (!Gen.IsValid())
		return;

	if (Gen->Lines.Num() == 0)
		return;

	FVector2D StartPosition = FVector2D::ZeroVector;
	// Calculate startPosition
	if (AdjustedPos < Gen->Characters.Num())
	{
		const FUICharInfo& CursorChar = Gen->Characters[AdjustedPos];
		StartPosition.X = CursorChar.CursorPos.X;
	}

	// TODO: Only clamp when Text uses horizontal word wrap.
	if (StartPosition.X > TextGraphicElem->GetTransformComponent()->GetRect().GetXMax())
		StartPosition.X = TextGraphicElem->GetTransformComponent()->GetRect().GetXMax();

	const int32 CharacterLine = DetermineCharacterLine(AdjustedPos, Gen);
	StartPosition.Y = Gen->Lines[CharacterLine].TopY;
	const float Height = Gen->Lines[CharacterLine].Height;

	for (int32 Index = 0, Count = CursorVerts.Num(); Index < Count; ++Index)
	{
		FastLinearColorToFColor(CaretColor, &CursorVerts[Index].Color);
	}

	CursorVerts[0].Position = FVector(StartPosition.X, StartPosition.Y - Height, 0.0f);
	CursorVerts[1].Position = FVector(StartPosition.X + Width, StartPosition.Y - Height, 0.0f);
	CursorVerts[2].Position = FVector(StartPosition.X + Width, StartPosition.Y, 0.0f);
	CursorVerts[3].Position = FVector(StartPosition.X, StartPosition.Y, 0.0f);

	if (RoundingOffset != FVector2D::ZeroVector)
	{
		for (int32 Index = 0, Count = CursorVerts.Num(); Index < Count; ++Index)
		{
			auto& UIV = CursorVerts[Index];
			UIV.Position.X += RoundingOffset.X;
			UIV.Position.Y += RoundingOffset.Y;
		}
	}

	VertexHelper.AddUIVertexQuad(CursorVerts);

	/*int screenHeight = Screen.height;
	// Multiple display support only when not the main display. For display 0 the reported
	// resolution is always the desktops resolution since its part of the display API,
	// so we use the standard none multiple display method. (case 741751)
	int displayIndex = m_TextComponent.canvas.targetDisplay;
	if (displayIndex > 0 && displayIndex < Display.displays.Length)
	    screenHeight = Display.displays[displayIndex].renderingHeight;

	startPosition.y = screenHeight - startPosition.y;
	input.compositionCursorPos = startPosition;*/
}

void UInputFieldComponent::GenerateHighlight(IGraphicElementInterface* TextGraphicElem, FVertexHelper& VertexHelper, FVector2D RoundingOffset) const
{
	int32 StartChar = FMath::Max(0, GetCaretPositionInternal() - DrawStart);
	int32 EndChar = FMath::Max(0, GetCaretSelectPositionInternal() - DrawStart);

	// Ensure pos is always less then selPos to make the code simpler
	if (StartChar > EndChar)
	{
		const int32 Temp = StartChar;
		StartChar = EndChar;
		EndChar = Temp;
	}

	EndChar -= 1;
	auto& Gen = TextComponent->GetCachedTextGenerator();

	if (Gen.Lines.Num() <= 0)
		return;

	int32 CurrentLineIndex = DetermineCharacterLine(StartChar, TextComponent->GetRawCachedTextGenerator());
	int32 LastCharInLineIndex = GetLineEndPosition(TextComponent->GetRawCachedTextGenerator(), CurrentLineIndex);

	FUIVertex Vert = FUIVertex::SimpleVertex;
	Vert.UV0 = FVector2D::ZeroVector;
	FastLinearColorToFColor(SelectionColor, &Vert.Color);
	
	const auto TextRect = TextGraphicElem->GetTransformComponent()->GetRect();

	int32 CurrentChar = StartChar;
	const int32 CharacterCount = Gen.Characters.Num();
	while (CurrentChar <= EndChar && CurrentChar < CharacterCount)
	{
		if (CurrentChar == LastCharInLineIndex || CurrentChar == EndChar)
		{
			FUICharInfo& StartCharInfo = Gen.Characters[StartChar];
			FUICharInfo& EndCharInfo = Gen.Characters[CurrentChar];
			const FVector2D StartPosition = FVector2D(StartCharInfo.CursorPos.X,
			                                    Gen.Lines[CurrentLineIndex].TopY);
			FVector2D EndPosition = FVector2D(
				(EndCharInfo.CursorPos.X + EndCharInfo.CharWidth),
				StartPosition.Y - Gen.Lines[CurrentLineIndex].Height);

			// Checking xMin as well due to text generator not setting position if char is not rendered.
			if (EndPosition.X > TextRect.GetXMax() || EndPosition.X < TextRect.XMin)
				EndPosition.X = TextRect.GetXMax();

			const int32 StartIndex = VertexHelper.GetCurrentVerticesCount();
			Vert.Position = FVector(StartPosition.X, EndPosition.Y, 0.0f) + FVector(RoundingOffset, 0);
			VertexHelper.AddVert(Vert);

			Vert.Position = FVector(EndPosition.X, EndPosition.Y, 0.0f) + FVector(RoundingOffset, 0);
			VertexHelper.AddVert(Vert);

			Vert.Position = FVector(EndPosition.X, StartPosition.Y, 0.0f) + FVector(RoundingOffset, 0);
			VertexHelper.AddVert(Vert);

			Vert.Position = FVector(StartPosition.X, StartPosition.Y, 0.0f) + FVector(RoundingOffset, 0);
			VertexHelper.AddVert(Vert);

			VertexHelper.AddTriangle(StartIndex, StartIndex + 1, StartIndex + 2);
			VertexHelper.AddTriangle(StartIndex + 2, StartIndex + 3, StartIndex + 0);

			StartChar = CurrentChar + 1;
			++CurrentLineIndex;

			LastCharInLineIndex = GetLineEndPosition(TextComponent->GetRawCachedTextGenerator(), CurrentLineIndex);
		}
		
		++CurrentChar;
	}
}

TCHAR UInputFieldComponent::Validate(const FString& TextString, int32 Pos, TCHAR Character) const
{
	// Validation is disabled
    if (CharacterValidation == EInputFieldCharacterValidation::None || !IsEnabled())
		return Character;

	if (CharacterValidation == EInputFieldCharacterValidation::Integer || CharacterValidation == EInputFieldCharacterValidation::Decimal)
	{
		// Integer and decimal
		const bool bCursorBeforeDash = (Pos == 0 && TextString.Len() > 0 && TextString[0] == TEXT('-'));
		const bool bDashInSelection = TextString.Len() > 0 && TextString[0] == TEXT('-') && ((GetCaretPositionInternal() == 0 &&
			GetCaretSelectPositionInternal() > 0) || (GetCaretSelectPositionInternal() == 0 && GetCaretPositionInternal() > 0));
		const bool bSelectionAtStart = GetCaretPositionInternal() == 0 || GetCaretSelectPositionInternal() == 0;
		if (!bCursorBeforeDash || bDashInSelection)
		{
			if (Character >= TEXT('0') && Character <= TEXT('9')) return Character;
			if (Character == TEXT('-') && (Pos == 0 || bSelectionAtStart)) return Character;

			TCHAR NumberSeparators[] = { TEXT('.'), TEXT(',') };
			if ((Character == TEXT('.') || Character == TEXT(',')) && CharacterValidation == EInputFieldCharacterValidation::Decimal &&
				IndexOfAny(TextString, NumberSeparators, 2, 0) == -1) return Character;		
		}
	}
	else if (CharacterValidation == EInputFieldCharacterValidation::Alphanumeric)
	{
		// All alphanumeric characters
		if (Character >= TEXT('A') && Character <= TEXT('Z')) return Character;
		if (Character >= TEXT('a') && Character <= TEXT('z')) return Character;
		if (Character >= TEXT('0') && Character <= TEXT('9')) return Character;
	}
	else if (CharacterValidation == EInputFieldCharacterValidation::Name)
	{
		// FIXME: some actions still lead to invalid input:
		//        - Hitting delete in front of an uppercase letter
		//        - Selecting an uppercase letter and deleting it
		//        - Typing some text, hitting Home and typing more text (we then have an uppercase letter in the middle of a word)
		//        - Typing some text, hitting Home and typing a space (we then have a leading space)
		//        - Erasing a space between two words (we then have an uppercase letter in the middle of a word)
		//        - We accept a trailing space
		//        - We accept the insertion of a space between two lowercase letters.
		//        - Typing text in front of an existing uppercase letter
		//        - ... and certainly more
		//
		// The rule we try to implement are too complex for this kind of verification.
		const bool bIsLetter = (Character >= TEXT('a') && Character <= TEXT('z')) || (Character >= TEXT('A') && Character <= TEXT('Z'));
		if (bIsLetter)
		{
			// Character following a space should be in uppercase.
			if (FChar::IsLower(Character) && ((Pos == 0) || (TextString[Pos - 1] == TEXT(' '))))
			{
				return FChar::ToUpper(Character);
			}

			// Character not following a space or an apostrophe should be in lowercase.
			if (FChar::IsUpper(Character) && (Pos > 0) && (TextString[Pos - 1] != TEXT(' ')) && (TextString[Pos - 1] != TEXT('\'')))
			{
				return FChar::ToLower(Character);
			}

			return Character;
		}

		if (Character == TEXT('\''))
		{
			// Don't allow more than one apostrophe
			if (!TextString.Contains(TEXT("'")))
				// Don't allow consecutive spaces and apostrophes.
				if (!(((Pos > 0) && ((TextString[Pos - 1] == TEXT(' ')) || (TextString[Pos - 1] == TEXT('\'')))) ||
					((Pos < TextString.Len()) && ((TextString[Pos] == TEXT(' ')) || (TextString[Pos] == TEXT('\''))))))
					return Character;
		}

		if (Character == TEXT(' '))
		{
			if (Pos != 0) // Don't allow leading spaces
			{
				// Don't allow consecutive spaces and apostrophes.
				if (!(((Pos > 0) && ((TextString[Pos - 1] == TEXT(' ')) || (TextString[Pos - 1] == TEXT('\'')))) ||
					((Pos < TextString.Len()) && ((TextString[Pos] == TEXT(' ')) || (TextString[Pos] == TEXT('\''))))))
					return Character;
			}
		}
	}
	else if (CharacterValidation == EInputFieldCharacterValidation::EmailAddress)
	{
		// From StackOverflow about allowed characters in email addresses:
		// Uppercase and lowercase English letters (a-z, A-Z)
		// Digits 0 to 9
		// Characters ! # $ % & ' * + - / = ? ^ _ ` { | } ~
		// Character . (dot, period, full stop) provided that it is not the first or last character,
		// and provided also that it does not appear two or more times consecutively.

		if (Character >= TEXT('A') && Character <= TEXT('Z')) return Character;
		if (Character >= TEXT('a') && Character <= TEXT('z')) return Character;
		if (Character >= TEXT('0') && Character <= TEXT('9')) return Character;

		int32 ResultIndex;
		if (Character == TEXT('@') && !TextString.FindChar(TEXT('@'), ResultIndex))
		{
			return Character;
		}
		
		if (EmailSpecialCharacters.FindChar(Character, ResultIndex))
		{
			return Character;
		}
		
		if (Character == TEXT('.'))
		{
			const TCHAR LastChar = (TextString.Len() > 0) ? TextString[FMath::Clamp(Pos, 0, TextString.Len() - 1)] : TEXT(' ');
			const TCHAR NextChar = (TextString.Len()  > 0) ? TextString[FMath::Clamp(Pos + 1, 0, TextString.Len() - 1)] : TEXT('\n');
			if (LastChar != TEXT('.') && NextChar != TEXT('.'))
				return Character;
		}
	}
	
    return 0;	
}

void UInputFieldComponent::ActivateInputField()
{
	if (!TextComponent || TextComponent->GetFont() == nullptr || !IsActiveAndEnabled() || !IsInteractableInHierarchy())
		return;

	if (IsFocused())
	{
		// TODO
		/*if (m_Keyboard != null && !m_Keyboard.active)
		{
			m_Keyboard.active = true;
			m_Keyboard.text = m_Text;
		}*/
	}

	bShouldActivateNextUpdate = true;
}

void UInputFieldComponent::ActivateInputFieldInternal()
{
	const auto World = GetWorld();
	const auto EventSystem = UEventSystemComponent::GetCurrentEventSystem(World);
	if (!IsValid(EventSystem))
		return;

	if (EventSystem->GetCurrentSelectedGameObject() != this)
	{
		EventSystem->SetSelectedGameObject(this);
	}

	if (FPlatformApplicationMisc::RequiresVirtualKeyboard())
	{
		/*if (input.touchSupported)
		{
			TouchScreenKeyboard.hideInput = shouldHideMobileInput;
		}*/

		FSlateApplication::Get().ShowVirtualKeyboard(true, 0, VirtualKeyboardEntry);

		// Cache the value of isInPlaceEditingAllowed, because on UWP this involves calling into native code
		// The value only needs to be updated once when the TouchKeyboard is opened.
		// m_TouchKeyboardAllowsInPlaceEditing = TouchScreenKeyboard.isInPlaceEditingAllowed;
		if (bUseIntegratedKeyboard)
		{
			bTouchKeyboardAllowsInPlaceEditing = true;
		}

		// Mimics OnFocus but as mobile doesn't properly support select all
		// just set it to the end of the text (where it would move when typing starts)
		MoveTextEnd(false);
	}
	else
	{
		ITextInputMethodSystem* const TextInputMethodSystem = FSlateApplication::Get().GetTextInputMethodSystem();
		if (TextInputMethodSystem)
		{
			if (!bHasRegisteredTextInputMethodContext)
			{
				bHasRegisteredTextInputMethodContext = true;
				TextInputMethodSystem->RegisterContext(TextInputMethodContext.ToSharedRef());
			}
			TextInputMethodSystem->ActivateContext(TextInputMethodContext.ToSharedRef());
		}
		
	}

	OnFocus();

	bAllowInput = true;
	OriginalText = Text;
	bWasCanceled = false;
	SetCaretVisible();
	UpdateLabel();
}

void UInputFieldComponent::OnSelect(UBaseEventData* EventData)
{
	Super::OnSelect(EventData);

	if (ShouldActivateOnSelect())
		ActivateInputField();
}

void UInputFieldComponent::OnPointerClick(UPointerEventData* EventData)
{
	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;

	ActivateInputField();
}

void UInputFieldComponent::DeactivateInputField()
{
	// Not activated do nothing.
	if (!bAllowInput)
		return;

	bHasDoneFocusTransition = false;
	bAllowInput = false;

	if (IsValid(Placeholder))
		Placeholder->SetEnabled(Text.IsEmpty());

	if (TextComponent && IsInteractableInHierarchy())
	{
		if (bWasCanceled)
			Text = OriginalText;

		SendOnSubmit();

		if(FSlateApplication::IsInitialized() && FPlatformApplicationMisc::RequiresVirtualKeyboard())
		{
			// TODO 要不要手动调用下VirtualKeyboard的函数呢
			FSlateApplication::Get().ShowVirtualKeyboard(false, 0);
		}

		if (TextInputMethodContext.IsValid())
		{
			TextInputMethodContext->EndComposition();
		}
		
		if (bHasRegisteredTextInputMethodContext)
		{
			ITextInputMethodSystem* const TextInputMethodSystem = FSlateApplication::IsInitialized() ? FSlateApplication::Get().GetTextInputMethodSystem() : nullptr;
			if (TextInputMethodSystem)
			{
				const TSharedRef<FTextInputMethodContext> TextInputMethodContextRef = TextInputMethodContext.ToSharedRef();
				if (TextInputMethodSystem->IsActiveContext(TextInputMethodContextRef))
				{
					TextInputMethodSystem->DeactivateContext(TextInputMethodContextRef);
				}
			}
		}
		
		CaretPosition = CaretSelectPosition = 0;
	}

	MarkGeometryAsDirty();
}

void UInputFieldComponent::OnDeselect(UBaseEventData* EventData)
{
	DeactivateInputField();
	Super::OnDeselect(EventData);
}

void UInputFieldComponent::OnSubmit(UBaseEventData* EventData)
{
	if (!IsActiveAndEnabled() || !IsInteractableInHierarchy())
		return;

	if (!IsFocused())
	{
		bShouldActivateNextUpdate = true;
	}
}

void UInputFieldComponent::EnforceContentType()
{
	switch (ContentType)
	{
		case EInputFieldContentType::Standard:
		{
			// Don't enforce line type for this content type.
			InputType = EInputFieldInputType::Standard;
			KeyboardType = ETouchScreenKeyboardType::Keyboard_Default;
			CharacterValidation = EInputFieldCharacterValidation::None;
			break;
		}
		case EInputFieldContentType::AutoCorrected:
		{
			// Don't enforce line type for this content type.
			InputType = EInputFieldInputType::AutoCorrect;
			KeyboardType = ETouchScreenKeyboardType::Keyboard_Default;
			CharacterValidation = EInputFieldCharacterValidation::None;
			break;
		}
		case EInputFieldContentType::IntegerNumber:
		{
			LineType = EInputFieldLineType::SingleLine;
			InputType = EInputFieldInputType::Standard;
			KeyboardType = ETouchScreenKeyboardType::Keyboard_Number;
			CharacterValidation = EInputFieldCharacterValidation::Integer;
			break;
		}
		case EInputFieldContentType::DecimalNumber:
		{
			LineType = EInputFieldLineType::SingleLine;
			InputType = EInputFieldInputType::Standard;
			KeyboardType = ETouchScreenKeyboardType::Keyboard_Number;
			CharacterValidation = EInputFieldCharacterValidation::Decimal;
			break;
		}
		case EInputFieldContentType::Alphanumeric:
		{
			LineType = EInputFieldLineType::SingleLine;
			InputType = EInputFieldInputType::Standard;
			KeyboardType = ETouchScreenKeyboardType::Keyboard_AlphaNumeric;
			CharacterValidation = EInputFieldCharacterValidation::Alphanumeric;
			break;
		}
		case EInputFieldContentType::Name:
		{
			LineType = EInputFieldLineType::SingleLine;
			InputType = EInputFieldInputType::Standard;
			KeyboardType = ETouchScreenKeyboardType::Keyboard_Default;
			CharacterValidation = EInputFieldCharacterValidation::Name;
			break;
		}
		case EInputFieldContentType::EmailAddress:
		{
			LineType = EInputFieldLineType::SingleLine;
			InputType = EInputFieldInputType::Standard;
			KeyboardType = ETouchScreenKeyboardType::Keyboard_Email;
			CharacterValidation = EInputFieldCharacterValidation::EmailAddress;
			break;
		}
		case EInputFieldContentType::Password:
		{
			LineType = EInputFieldLineType::SingleLine;
			InputType = EInputFieldInputType::Password;
			KeyboardType = ETouchScreenKeyboardType::Keyboard_Password;
			CharacterValidation = EInputFieldCharacterValidation::None;
			break;
		}
		case EInputFieldContentType::Pin:
		{
			LineType = EInputFieldLineType::SingleLine;
			InputType = EInputFieldInputType::Password;
			KeyboardType = ETouchScreenKeyboardType::Keyboard_Number;
			CharacterValidation = EInputFieldCharacterValidation::Integer;
			break;
		}
		default:
		{
			// Includes Custom type. Nothing should be enforced.
			break;
		}
	}

	EnforceTextHOverflow();
}

void UInputFieldComponent::EnforceTextHOverflow() const
{
	if (TextComponent)
	{
		if (IsMultiLine())
		{
			TextComponent->SetHorizontalOverflow(EHorizontalWrapMode::HorizontalWrapMode_Wrap);
		}
		else
		{
			TextComponent->SetHorizontalOverflow(EHorizontalWrapMode::HorizontalWrapMode_Overflow);
		}
	}
}

void UInputFieldComponent::SetToCustomIfContentTypeIsNot(TArray<EInputFieldContentType, TInlineAllocator<8>> AllowedContentType)
{
	if (ContentType == EInputFieldContentType::Custom)
		return;

	for (int32 Index = 0, Count = AllowedContentType.Num(); Index < Count; ++Index)
	{
		if (ContentType == AllowedContentType[Index])
			return;
	}

	ContentType = EInputFieldContentType::Custom;
}

void UInputFieldComponent::SetToCustom()
{
	if (ContentType == EInputFieldContentType::Custom)
		return;

	ContentType = EInputFieldContentType::Custom;
}

void UInputFieldComponent::DoStateTransition(ESelectableSelectionState InState, bool bInstant)
{
	if (bHasDoneFocusTransition)
	{
		InState = ESelectableSelectionState::SelectionState_Highlighted;
	}
	else if (InState == ESelectableSelectionState::SelectionState_Pressed)
	{
		bHasDoneFocusTransition = true;
	}
	
	Super::DoStateTransition(InState, bInstant);
}

float UInputFieldComponent::GetPreferredWidth()
{
	if (!TextComponent)
		return 0;

	const FTextGenerationSettings Settings = TextComponent->GetGenerationSettings(FVector2D::ZeroVector);
	return TextComponent->GetCachedTextGeneratorForLayout().GetPreferredWidth(Text.ToString(), Settings, TextComponent.GetObject());
}

float UInputFieldComponent::GetPreferredHeight()
{
	if (!TextComponent)
		return 0;

	const FTextGenerationSettings Settings = TextComponent->GetGenerationSettings(FVector2D(GetRect().GetSize().X, 0));
	return TextComponent->GetCachedTextGeneratorForLayout().GetPreferredHeight(Text.ToString(), Settings, TextComponent.GetObject());
}

TSharedRef<UInputFieldComponent::FVirtualKeyboardEntry> UInputFieldComponent::FVirtualKeyboardEntry::Create(
	UInputFieldComponent* InInputField)
{
	return MakeShareable(new FVirtualKeyboardEntry(InInputField));
}

UInputFieldComponent::FVirtualKeyboardEntry::FVirtualKeyboardEntry(UInputFieldComponent* InInputField)
	: InputField(InInputField)
{
}

void UInputFieldComponent::FVirtualKeyboardEntry::SetTextFromVirtualKeyboard(const FText& InNewText,
	ETextEntryType TextEntryType)
{
	check(IsInGameThread());

	/*// Only set the text if the text attribute doesn't have a getter binding (otherwise it would be blown away).
	// If it is bound, we'll assume that OnTextCommitted will handle the update.
	if (!OwnerLayout->BoundText.IsBound())
	{
		OwnerLayout->BoundText.Set(InNewText);
	}

	// Update the internal editable text
	// This method is called from the main thread (i.e. not the game thread) of the device with the virtual keyboard
	// This causes the app to crash on those devices, so we're using polling here to ensure delegates are
	// fired on the game thread in Tick.		
	OwnerLayout->VirtualKeyboardText = InNewText;
	OwnerLayout->bTextChangedByVirtualKeyboard = true;
	if (TextEntryType == ETextEntryType::TextEntryAccepted)
	{
		if (OwnerLayout->OwnerWidget->GetVirtualKeyboardDismissAction() == EVirtualKeyboardDismissAction::TextCommitOnAccept || 
			OwnerLayout->OwnerWidget->GetVirtualKeyboardDismissAction() == EVirtualKeyboardDismissAction::TextCommitOnDismiss)
		{
			OwnerLayout->VirtualKeyboardTextCommitType = ETextCommit::OnEnter;
			OwnerLayout->bTextCommittedByVirtualKeyboard = true;
		}
	}
	else if (TextEntryType == ETextEntryType::TextEntryCanceled)
	{
		if (OwnerLayout->OwnerWidget->GetVirtualKeyboardDismissAction() == EVirtualKeyboardDismissAction::TextCommitOnDismiss)
		{
			OwnerLayout->VirtualKeyboardTextCommitType = ETextCommit::Default;
			OwnerLayout->bTextCommittedByVirtualKeyboard = true;
		}
	}*/
	// TODO
	if (InputField.IsValid())
	{
		InputField->SetText(InNewText);
	}
}

void UInputFieldComponent::FVirtualKeyboardEntry::SetSelectionFromVirtualKeyboard(int InSelStart, int InSelEnd)
{
	check(IsInGameThread());
}

bool UInputFieldComponent::FVirtualKeyboardEntry::GetSelection(int& OutSelStart, int& OutSelEnd)
{
	check(IsInGameThread());
	return true;
}

FText UInputFieldComponent::FVirtualKeyboardEntry::GetText() const
{
	check(IsInGameThread());

	if (InputField.IsValid())
	{
		return InputField->GetText();
	}
	return FText::GetEmpty();
}

FText UInputFieldComponent::FVirtualKeyboardEntry::GetHintText() const
{
	check(IsInGameThread());
	
	if (InputField.IsValid())
	{
		if (IsValid(InputField->Placeholder))
		{
			const auto TextElem = Cast<ITextElementInterface>(InputField->Placeholder->GetComponentByInterface(UTextElementInterface::StaticClass(), true));
			if (TextElem)
			{
				return TextElem->GetText();
			}
		}
	}
	return FText::GetEmpty();
}

EKeyboardType UInputFieldComponent::FVirtualKeyboardEntry::GetVirtualKeyboardType() const
{
	check(IsInGameThread());
	
	if (InputField.IsValid())
	{
		return static_cast<EKeyboardType>(InputField->KeyboardType);
	}
	return EKeyboardType::Keyboard_Default;
}

FVirtualKeyboardOptions UInputFieldComponent::FVirtualKeyboardEntry::GetVirtualKeyboardOptions() const
{
	check(IsInGameThread());

	FVirtualKeyboardOptions VirtualKeyboardOptions;
	VirtualKeyboardOptions.bEnableAutocorrect = false;
	if (InputField.IsValid())
	{
		VirtualKeyboardOptions.bEnableAutocorrect = InputField->InputType == EInputFieldInputType::AutoCorrect;
	}
	return VirtualKeyboardOptions;
}

bool UInputFieldComponent::FVirtualKeyboardEntry::IsMultilineEntry() const
{
	check(IsInGameThread());
	
	if (InputField.IsValid())
	{
		return InputField->IsMultiLine();
	}
	return false;
}

TSharedRef<UInputFieldComponent::FTextInputMethodContext> UInputFieldComponent::FTextInputMethodContext::Create(
	UInputFieldComponent* InInputField)
{
	return MakeShareable(new FTextInputMethodContext(InInputField));
}

UInputFieldComponent::FTextInputMethodContext::FTextInputMethodContext(UInputFieldComponent* InInputField)
	: InputField(InInputField)
{
	
}

void UInputFieldComponent::FTextInputMethodContext::KillContext()
{
	InputField = nullptr;
	bIsComposing = false;
	CompositionBeginIndex = 0;
	CompositionLength = 0;
	
	if (CachedWindow.IsValid() && GEngine && GEngine->IsValidLowLevelFast()
		&& GEngine->GameViewport && GEngine->GameViewport->IsValidLowLevelFast())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(CachedWindow.ToSharedRef());
	}
	CachedWindow.Reset();
}

bool UInputFieldComponent::FTextInputMethodContext::IsComposing()
{
	return InputField.IsValid() && bIsComposing;
}

bool UInputFieldComponent::FTextInputMethodContext::IsReadOnly()
{
	return !InputField.IsValid() || InputField->IsReadOnly();
}

uint32 UInputFieldComponent::FTextInputMethodContext::GetTextLength()
{
	if (!InputField.IsValid())
	{
		return 0;
	}
	return InputField->CompositionString.Len();
}

void UInputFieldComponent::FTextInputMethodContext::GetSelectionRange(uint32& BeginIndex, uint32& Length,
	ECaretPosition& InCaretPosition)
{
	BeginIndex = CompositionBeginIndex;
	Length = CompositionLength;
	InCaretPosition = ITextInputMethodContext::ECaretPosition::Ending;
}

void UInputFieldComponent::FTextInputMethodContext::SetSelectionRange(const uint32 BeginIndex, const uint32 Length,
	const ECaretPosition InCaretPosition)
{
	CompositionBeginIndex = BeginIndex;
	CompositionLength = Length;
}

void UInputFieldComponent::FTextInputMethodContext::GetTextInRange(const uint32 BeginIndex, const uint32 Length,
	FString& OutString)
{
	if (InputField.IsValid())
	{
		OutString = InputField->CompositionString;
	}
}

void UInputFieldComponent::FTextInputMethodContext::SetTextInRange(const uint32 BeginIndex, const uint32 Length,
	const FString& InString)
{
	if (InputField.IsValid())
	{
		InputField->CompositionString.RemoveAt(BeginIndex, Length, false);
		InputField->CompositionString.InsertAt(BeginIndex, InString);
		InputField->UpdateLabel();
	}
}

int32 UInputFieldComponent::FTextInputMethodContext::GetCharacterIndexFromPoint(const FVector2D& Point)
{
	return 0;
}

bool UInputFieldComponent::FTextInputMethodContext::GetTextBounds(const uint32 BeginIndex, const uint32 Length,
	FVector2D& Position, FVector2D& Size)
{
	Position = FVector2D(0, 0);
	Size = FVector2D(1000, 1000);
	//UE_LOG(LogUGUI, Error, TEXT("===========zed====GetTextBounds:%s"), *(Position.ToString()));
	return false;
}

void UInputFieldComponent::FTextInputMethodContext::GetScreenBounds(FVector2D& Position, FVector2D& Size)
{
	Position = FVector2D(0, 0);
	Size = FVector2D(1000, 1000);
	//UE_LOG(LogUGUI, Error, TEXT("===========zed====GetScreenBounds, Position:%s, Size:%s"), *(Position.ToString()), *(Size.ToString()));
}

TSharedPtr<FGenericWindow> UInputFieldComponent::FTextInputMethodContext::GetWindow()
{
	if (!CachedWindow.IsValid() && GEngine && GEngine->IsValidLowLevelFast()
		&& GEngine->GameViewport && GEngine->GameViewport->IsValidLowLevelFast())
	{
		CachedWindow = SNew(SBox);
		GEngine->GameViewport->AddViewportWidgetContent(CachedWindow.ToSharedRef());
	}
	
	const TSharedPtr<SWindow> SlateWindow = FSlateApplication::Get().FindWidgetWindow(CachedWindow.ToSharedRef());
	return SlateWindow->GetNativeWindow();
}

void UInputFieldComponent::FTextInputMethodContext::BeginComposition()
{
	if (!bIsComposing)
	{
		bIsComposing = true;
	}
}

void UInputFieldComponent::FTextInputMethodContext::UpdateCompositionRange(const int32 InBeginIndex,
	const uint32 InLength)
{
	if (bIsComposing)
	{
		CompositionBeginIndex = InBeginIndex;
		CompositionLength = InLength;
	}
}

void UInputFieldComponent::FTextInputMethodContext::EndComposition()
{
	CompositionBeginIndex = 0;
	CompositionLength = 0;
	
	if (bIsComposing)
	{
		bIsComposing = false;
	}

	if (InputField.IsValid())
	{
		const FString TextString = MoveTemp(InputField->CompositionString);
		InputField->CompositionString.Empty();
		InputField->Append(TextString);
		InputField->UpdateLabel();
	}
}

/////////////////////////////////////////////////////
