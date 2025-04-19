#include "Sprite.hlsli"
Texture2D colorMap : register(t0);
SamplerState samplerStates : register(s0);

float4 main(PSIn psIn) : SV_TARGET
{
    return colorMap.Sample(samplerStates, psIn.texcoord) * psIn.color;
}