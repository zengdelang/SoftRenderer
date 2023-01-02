#pragma once

#include "CoreMinimal.h"
#include "TweenDefines.h"
#include "Core/Layout/RectTransformComponent.h"
#include "TweenBaseSubComponent.h"
#include "TweenPlayerSubComponent.h"
#include "TweenUtility.generated.h"

class UGUI_API FTweenUtility
{
public:

	/**
	 * Interpolate a linear alpha value using an ease mode and function.
	 */
	static float EaseAlpha(ETweenEasingFunc InEasingFunc, float InAlpha, float BlendExp = 2);

	static UTweenPlayerSubComponent* GetTweenPlayer(URectTransformComponent* RectTransform);

	static UTweenBaseSubComponent* GetTween(URectTransformComponent* RectTransform, const FString& TweenGroupName, int32 TweenGroupIndex);

	static void PlayTween(URectTransformComponent* RectTransform, const FString& TweenGroupName, int32 TweenGroupIndex);

	static void PlayTweenGroup(URectTransformComponent* RectTransform, const FString& TweenGroupName);

	static FVector ScreenPointToLocalPoint(URectTransformComponent* RectTransform, UCanvasSubComponent* Canvas, const FVector& ScreenPosition, UWorld* World);

};

UCLASS()
class UGUI_API UTweenLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = TweenUtility)
	static UTweenPlayerSubComponent* GetTweenPlayer(URectTransformComponent* RectTransform);

	UFUNCTION(BlueprintCallable, Category = TweenUtility)
	static UTweenBaseSubComponent* GetTween(URectTransformComponent* RectTransform, FString TweenGroupName, int32 TweenGroupIndex);

	UFUNCTION(BlueprintCallable, Category = TweenUtility)
	static void PlayTween(URectTransformComponent* RectTransform, FString TweenGroupName, int32 TweenGroupIndex);

	UFUNCTION(BlueprintCallable, Category = TweenUtility)
	static void PlayTweenGroup(URectTransformComponent* RectTransform, FString TweenGroupName);
};