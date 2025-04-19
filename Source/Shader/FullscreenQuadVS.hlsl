#include "FullscreenQuad.hlsli"

PSIn main(in uint vertexID : SV_VERTEXID)
{
    PSIn psIn;
    const float2 position[4] = { { -1, +1 }, { +1, +1 }, { -1, -1 }, { +1, -1 } };
    const float2 texcoords[4] = { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } };
    psIn.position_ = float4(position[vertexID], 0, 1);
    psIn.texcoord_ = texcoords[vertexID];
    
    return psIn;
}