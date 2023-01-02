#include "Core/Renderer/UIPostProcessShaders.h"

IMPLEMENT_TYPE_LAYOUT(FUIPostProcessElementShader);

IMPLEMENT_SHADER_TYPE(, FUIPostProcessDownsamplePS, TEXT("/Plugin/UGUI/Private/UIPostProcessShader.usf"), TEXT("DownsampleMain"), SF_Pixel);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FUIScreenVS, TEXT("/Plugin/UGUI/Private/UIPostProcessShader.usf"), TEXT("MainScreenVS"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FUIPostProcessElementVS, TEXT("/Plugin/UGUI/Private/UIPostProcessShader.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FUIPostProcessElementPS, TEXT("/Plugin/UGUI/Private/UIPostProcessShader.usf"), TEXT("MainPS"), SF_Pixel);

// Gaussian Blur
IMPLEMENT_SHADER_TYPE(, FUIBlurPostProcessBlurPS, TEXT("/Plugin/UGUI/Private/UIGaussianBlurShader.usf"), TEXT("GaussianBlurMain"), SF_Pixel);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FUIBlurPostProcessElementVS, TEXT("/Plugin/UGUI/Private/UIGaussianBlurShader.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FUIBlurPostProcessElementPS, TEXT("/Plugin/UGUI/Private/UIGaussianBlurShader.usf"), TEXT("MainPS"), SF_Pixel);

// Glitch
IMPLEMENT_SHADER_TYPE(, FUIGlitchPostProcessGlitchPS, TEXT("/Plugin/UGUI/Private/UIGlitchShader.usf"), TEXT("GlitchMain"), SF_Pixel);
