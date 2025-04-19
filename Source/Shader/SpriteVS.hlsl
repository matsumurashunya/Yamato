#include "Sprite.hlsli"

PSIn main(float position : POSITIONT, float4 color : COLOR, float2 texcoord : TEXCOORD)
{
    PSIn vsOut;
    
    vsOut.position = position;
    vsOut.color    = color;
    vsOut.texcoord = texcoord;
    
    return vsOut;
}