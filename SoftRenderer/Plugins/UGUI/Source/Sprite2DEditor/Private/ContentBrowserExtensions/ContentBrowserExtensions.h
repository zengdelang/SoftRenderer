#pragma once

#include "CoreMinimal.h"

// Integrate Sprite actions associated with existing engine types (e.g., Texture2D) into the content browser
class FSprite2DContentBrowserExtensions
{
public:
	static void InstallHooks();
	static void RemoveHooks();
	
};
