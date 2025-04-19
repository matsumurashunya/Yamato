#include "Model.hlsli"

struct TextureInfo
{
    int index_;
    int texcoord_;
};

struct NormalTextureInfo
{
    int index_;
    int texcoord_;
    float scale_;
};

struct OcclusionTextureInfo
{
    int index_;
    int texcoord_;
    float strength_;
};

struct PbrMetallicRoughenss
{
    float4 basecolorFactore_;
    TextureInfo basecolorTexture_;
    float roughnessFactor_;
    TextureInfo metallicRoughnessTexture_;
};

struct MaterialConstants
{
    float3 emissiveFactor_;
    int alphaMode_; // "OPAQUE" : 0, "MASK" : 1, "BLEND" : 2
    float alphaCutoff_;
    bool doubleSided_;
    
    PbrMetallicRoughenss pbrMetallicRoughness_;
    
    NormalTextureInfo normalTexture_;
    OcclusionTextureInfo occlusionTexutre_;
    TextureInfo emissiveTexuter_;
};
StructuredBuffer<MaterialConstants> materials : register(t0);

#define BASECOLOR_TEXTURE           0
#define METALLIC_ROUGHNESS_TEXTURE  1
#define NORMAL_TEXTURE              2
#define EMISSIVE_TEXTURE            3
#define OCCLUSION_TEXTURE           4
Texture2D<float4> materialTextures[5] : register(t1);

#define POINT       0
#define LINEAR      1
#define ANISOTROPIC 2
SamplerState samplerStates[3] : register(s0);


//PSIn main(VSIn vsIn) : SV_TARGET
float4 main(PSIn psIn) : SV_TARGET
{
    //float sigma = vsIn.tangent_.w;
    
    //if(skin_ >-1)
    //{
    //    row_major float4x4 skinMatrix =
    //    vsIn.weights_.x * jointMatrices[vsIn.joints_.x] +
    //    vsIn.weights_.y * jointMatrices[vsIn.joints_.y] +
    //    vsIn.weights_.z * jointMatrices[vsIn.joints_.z] +
    //    vsIn.weights_.w * jointMatrices[vsIn.joints_.w];
    //    vsIn.position_ = mul(float4(vsIn.position_.xyz, 1), skinMatrix);
    //    vsIn.normal_ = normalize(mul(float4(vsIn.position_.xyz, 0), skinMatrix));
    //    vsIn.tangent_ = normalize(mul(float4(vsIn.tangent_.xyz, 0), skinMatrix));
    //}
    
    //PSIn psIn;
    //vsIn.position_.w = 1;
    //psIn.position_ = mul(vsIn.position_, mul(world_, viewProjection_));
    //psIn.worldPosition_ = mul(vsIn.position_, world_);
    //vsIn.normal_.w = 0;
    //psIn.worldNormmal_ = normalize(mul(vsIn.normal_, world_));
    //vsIn.tangent_.w = 0;
    //psIn.worldTangent_ = normalize(mul(vsIn.tangent_, world_));
    //psIn.worldTangent_.w = sigma;
    //psIn.texcoord_ = vsIn.texcoord_;
    //return psIn;
    
    MaterialConstants materialConstants = materials[material_];
    
    float4 basecolor = materialConstants.pbrMetallicRoughness_.basecolorTexture_.index_ > -1 ?
        materialTextures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], psIn.texcoord_) :
        materialConstants.pbrMetallicRoughness_.basecolorFactore_;
    
    float3 emissive = materialConstants.emissiveTexuter_.index_ > -1 ?
    materialTextures[EMISSIVE_TEXTURE].Sample(samplerStates[ANISOTROPIC], psIn.texcoord_).rgb :
    materialConstants.emissiveFactor_;
    
    float3 N = normalize(psIn.worldNormmal_.xyz);
    float3 L = normalize(-lightDirection_.xyz);
    
    float3 color = max(0, dot(N, L)) * basecolor.rgb + emissive;
    return float4(color, basecolor.a);
}