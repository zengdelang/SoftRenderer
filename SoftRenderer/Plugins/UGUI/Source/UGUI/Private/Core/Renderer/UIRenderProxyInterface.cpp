#include "Core/Renderer/UIRenderProxyInterface.h"
#include "Core/Render/CanvasRendererSubComponent.h"

void IUIRenderProxyInterface::SetInstructionIndex(int32 InMinInstructionIndex, int32 InMaxInstructionIndex)
{
	MinInstructionIndex = InMinInstructionIndex;
	MaxInstructionIndex = InMaxInstructionIndex;
	BatchDescIndex = -1;
}
