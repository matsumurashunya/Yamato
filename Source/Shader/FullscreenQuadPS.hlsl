#include "FullscreenQuad.hlsli"

#define POINT           0
#define LINEAR          1 
#define ANISOTROPIC     2
SamplerState samplerStates[3] : register(s0);
Texture2D textureMap : register(t0);

float4 main(PSIn psIn) : SV_TARGET
{
    return textureMap.Sample(samplerStates[LINEAR], psIn.texcoord_);
}