#include "EventSystem/UGUIInputSubsystem.h"
#include "EventSystem/UGUIGameViewportClient.h"
#include "UGUI.h"

/////////////////////////////////////////////////////
// UUGUIInputSubsystem

UUGUIInputSubsystem::UUGUIInputSubsystem()
{
 
}

void UUGUIInputSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const auto& GameInstance = GetGameInstance();
	if (IsValid(GameInstance) && FApp::CanEverRender())
	{
		const auto& Engine = GameInstance->GetEngine();
		if (IsValid(Engine))
		{
			if (Engine->GameViewportClientClass == UGameViewportClient::StaticClass())
			{
				Engine->GameViewportClientClass = UUGUIGameViewportClient::StaticClass();
			}
			else if (Engine->GameViewportClientClass != UUGUIGameViewportClient::StaticClass())
			{		
				UE_LOG(LogUGUI, Warning, TEXT("Gameviewportclientclass needs to inherit from UUGUIGameViewportClient, otherwise the input system of UGUI will not take effect."));
			}
		}
	}
}

/////////////////////////////////////////////////////
