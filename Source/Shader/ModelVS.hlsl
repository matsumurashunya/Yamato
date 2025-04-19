#include "Model.hlsli"

PSIn main(VSIn vsIn)
{
    PSIn psIn;
    
    vsIn.position_.w = 1;
    psIn.position_ = mul(vsIn.position_, mul(world_, viewProjection_));
    psIn.worldPosition_ = mul(vsIn.position_, world_);
    
    vsIn.normal_.w = 0;
    psIn.worldNormmal_ = normalize(mul(vsIn.normal_, world_));
    
    float sigma = vsIn.tangent_.w;
    vsIn.tangent_.w = 0;
    psIn.worldTangent_ = normalize(mul(vsIn.tangent_, world_));
    psIn.worldTangent_.w = sigma;
    
    psIn.texcoord_ = vsIn.texcoord_;
    
    return psIn;
}