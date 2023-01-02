// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ITweenValue.h"
#include "UObject/NoExportTypes.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnRectTransformCallback, FVector2D);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCreateNewRectCallback, FVector2D);
/**
 * 
 */
class UGUI_API URectTransformMove : public ITweenValue
{
public:
	FVector2D StartPosition;
	FVector2D TargetPosition;

	FOnRectTransformCallback OnRectTransformCallback;
	FOnCreateNewRectCallback OnCreateNewRectCallback;
	
public:
	URectTransformMove()
	{
		StartPosition = FVector2D(0.0f, 0.0f);
		TargetPosition = FVector2D(0.0f, 0.0f);
	}

	URectTransformMove(FVector2D InStartPosition,FVector2D InTargetPosition)
	{
		StartPosition = InStartPosition;
		TargetPosition = InTargetPosition;
	}

	virtual void TweenValue(float InPercentage) override
	{
		FVector2D NewVector = FMath::Lerp(StartPosition, TargetPosition, InPercentage);
		OnRectTransformCallback.Broadcast(NewVector);
		
	}

};
