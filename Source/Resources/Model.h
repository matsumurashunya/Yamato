#pragma once
#define NOMINMAX
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"

class Model
{
public:
    struct Scene
    {
        std::string         name_;
        std::vector<int>    nodes_;
    };

    struct Node
    {
        std::string name_;
        int skin_ = 1;
        int mesh_ = 1;

        std::vector<int> children_;

        // ÉçÅ[ÉJÉã
        DirectX::XMFLOAT4 rotation_     = { 0, 0, 0, 1 };
        DirectX::XMFLOAT3 scale_        = { 1,1,1 };
        DirectX::XMFLOAT3 translation_  = { 0,0,0 };

        DirectX::XMFLOAT4X4 globalTransform_ =
        {
          1,0,0,0,
          0,1,0,0,
          0,0,1,0,
          0,0,0,1
        };
    };

    struct BufferView
    {
        DXGI_FORMAT format_ = DXGI_FORMAT_UNKNOWN;
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_;
        size_t strideInBytes_   = 0;
        size_t sizeInBytes_     = 0;
        size_t Count() const
        {
            return sizeInBytes_ / strideInBytes_;
        }
    };

    struct Mesh
    {
        std::string name_;
        struct Primitive
        {
            int material_;
            std::map<std::string, BufferView> vertexBufferViews_;
            BufferView indexBufferView_;
        };
        std::vector<Primitive> primitives_;
    };

    struct TextureInfo
    {
        int index_ = -1;
        int texcoord_ = 0;
    };

    struct NormalTexture
    {
        int index_ = -1;
        int texcoord_ = 0;
        float scale_ = 1;
    };

    struct OcclusionTextureInfo
    {
        int index_ = -1;
        int texcoord_ = 0;
        float strength_ = 1;
    };

    struct PbrMetallicRoughness
    {
        float basecolorFactor_[4] = { 1,1,1,1 };
        TextureInfo basecolorTexture_;
        float metallicFactor_ = 1;
        float roughnessFactor_ = 1;
        TextureInfo metallicRoughenssTexture_;
    };

    struct Material
    {
        std::string name_;
        struct Cbuffer
        {
            float emissiveFactor_[3] = { 0,0,0 };
            int alphaMode_ = 0;         // "OPAQUE" : 0, "MASK" : 1, "BLEND" : 2
            float alphaCutoff_ = 0.5f;
            bool doubleSided_ = false;

            PbrMetallicRoughness pbrMetallicRoughness_;

            NormalTexture normalTexture_;
            OcclusionTextureInfo occlusionTexture_;
            TextureInfo emissiveTexture_;
        };
        Cbuffer data_;
    };

    struct Texture
    {
        std::string name_;
        int source_ = -1;
    };

    struct Image
    {
        std::string name_;
        int width_ = -1;
        int height_ = -1;
        int component_ = -1;
        int bits_ = -1;
        int pixelType_ = -1;
        int bufferView_;
        std::string mimeType_;
        std::string uri_;
        bool asIs_ = false;
    };

    struct Skin
    {
        std::vector<DirectX::XMFLOAT4X4> inverseBindMatrices_;
        std::vector<int> joints_;
    };

    struct Animation
    {
        std::string name_;
        float duration_ = 0.0f;
        struct Channel
        {
            int sampler_ = -1;
            int targetNode_ = 01;
            std::string targetPath_;
        };
        std::vector<Channel> channels_;

        struct Sampler
        {
            int input_ = -1;
            int output_ = -1;
            std::string interpolation_;
        };
        std::vector<Sampler> samplers_;


        std::unordered_map<int/*sampler.input*/, std::vector<float>> timelines_;
        std::unordered_map<int/*sampler.output*/, std::vector<DirectX::XMFLOAT3>> scales_;
        std::unordered_map<int/*sampler.output*/, std::vector<DirectX::XMFLOAT4>> rotations_;
        std::unordered_map<int/*sampler.output*/, std::vector<DirectX::XMFLOAT3>> translations_;
    };

    static const size_t PRIMITIVE_MAX_JOINTS = 512;
    struct PrimitiveJointConstants
    {
        DirectX::XMFLOAT4X4 matrices[PRIMITIVE_MAX_JOINTS];
    };

public:
    Model(const std::string& filename);
    virtual ~Model() = default;

    void Render(const DirectX::XMFLOAT4X4 world, const std::vector<Node>& animatedNodes);

    void Animate(size_t animationIndex, float time, std::vector<Node>& animatedNodes);

private:
    struct PrimitiveConstants
    {
        DirectX::XMFLOAT4X4 world_;
        int material_       = -1;
        int hasTangent_     = 0;
        int skin_           = -1;
        int pad_;
    };

private:
    void FetchNodes(const tinygltf::Model& gltfModel);
    void FetchMeshes(const tinygltf::Model& gltfModel);
    void FetchMaterials(const tinygltf::Model& gltfModel);
    void FetchTexutres(const tinygltf::Model& gltfModel);
    void FetchAnimations(const tinygltf::Model& gltfModel);
    void CumulateTransforms(std::vector<Node>& nodes);

    BufferView MakeBufferView(const tinygltf::Accessor& accessor);

private:
    public:

    std::string filename_;

    std::vector<Scene> scenes_;
    std::vector<Node> nodes_;
    std::vector<Mesh> meshes_;
    std::vector<Material> materials_;
    std::vector<Texture> textures_;
    std::vector<Image> images_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> materialResourceView_;
    std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureResourceViews_;
    std::vector<Skin> skins_;
    std::vector<Animation> animations_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> primitiveJointCbuffer_;
    //Microsoft::WRL::ComPtr<ID3D11Buffer> primitiveJoint

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;

    Microsoft::WRL::ComPtr<ID3D11Buffer> primitiveCbuffer_;
};

