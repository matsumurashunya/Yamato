struct VSIn
{
    float4 position_ : POSITION;
    float4 normal_ : NORMAL;
    float4 tangent_ : TANGENT;
    float2 texcoord_ : TEXCOORD;
    uint4 joints_ : JOINTS;
    float4 weights_ : WEIGHTS;
};

struct PSIn
{
    float4 position_ : SV_POSITION;
    float4 worldPosition_ : POSITION;
    float4 worldNormmal_ : NORMAL;
    float4 worldTangent_ : TANGENT;
    float2 texcoord_ : TEXCOORD;
};

cbuffer PrimitiveConstantBuffer : register(b0)
{
    row_major float4x4 world_;
    int material_;
    bool hasTangent_;
    int skin_;
    int pad_;
}

cbuffer SceneConstant : register(b1)
{
    row_major float4x4 viewProjection_;
    float4 lightDirection_;
    float4 cameraPosition_;
}

static const uint PRIMITIVE_MAX_JOINTS = 512;
cbuffer PRIMITIVE_JOINNT_CONSTANTS : register(b2)
{
    row_major float4x4 jointMatrices[PRIMITIVE_MAX_JOINTS];
};