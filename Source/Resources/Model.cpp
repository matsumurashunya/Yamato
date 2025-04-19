#include "Model.h"
#define TINYGLTF_IMPLEMENTATION
#include "Graphics.h"
#include <stack>
#include "Texture.h"
#include <filesystem>
#include "tinygltf/tiny_gltf.h"

bool NullLoadImageData(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*)
{
    return true;
}

// コンストラクタ
Model::Model(const std::string& filename) : filename_(filename)
{
    tinygltf::TinyGLTF tinyGltf;
    tinyGltf.SetImageLoader(NullLoadImageData, nullptr);

    tinygltf::Model gltfModel;
    std::string error, warning;
    bool succeeded = false;

    if (filename.find(".glb") != std::string::npos)
    {
        succeeded = tinyGltf.LoadBinaryFromFile(&gltfModel, &error, &warning, filename.c_str());
    }
    else if (filename.find(".gltf") != std::string::npos)
    {
        succeeded = tinyGltf.LoadASCIIFromFile(&gltfModel, &error, &warning, filename.c_str());
    }

    _ASSERT_EXPR_A(warning.empty(), warning.c_str());
    _ASSERT_EXPR_A(error.empty(), error.c_str());
    _ASSERT_EXPR_A(succeeded, L"Failed to load gltf file.");

    for (std::vector<tinygltf::Scene>::const_reference gltfScene : gltfModel.scenes)
    {
        Scene& scene = scenes_.emplace_back();
        scene.name_ = gltfScene.name;
        scene.nodes_ = gltfScene.nodes;
    }

    FetchNodes(gltfModel);
    FetchMeshes(gltfModel);
    FetchMaterials(gltfModel);
    FetchTexutres(gltfModel);
    FetchAnimations(gltfModel);

    const std::map<std::string, BufferView>& vertexBufferViews = meshes_.at(0).primitives_.at(0).vertexBufferViews_;
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
    {
        { "POSITION", 0, vertexBufferViews.at("POSITION").format_,      0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "NORMAL"  , 0, vertexBufferViews.at("NORMAL").format_,        1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "TANGENT" , 0, vertexBufferViews.at("TANGENT").format_,       2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 0, vertexBufferViews.at("TEXCOORD_0").format_,    3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "JOINTS"  , 0, vertexBufferViews.at("JOINTS_0").format_,      4, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "WEIGHTS" , 0, vertexBufferViews.at("WEIGHTS_0").format_,     5, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    Graphics::GetInstance().GetShader()->CreateVsFromCso("./Resources/Shader/ModelVS.cso", vertexShader_.ReleaseAndGetAddressOf(), inputLayout_.ReleaseAndGetAddressOf(), inputElementDesc, _countof(inputElementDesc));
    Graphics::GetInstance().GetShader()->CreatePsFromCso("./Resources/Shader/ModelPS.cso", pixelShader_.ReleaseAndGetAddressOf());

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(PrimitiveConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    HRESULT result;
    result = Graphics::GetInstance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, primitiveCbuffer_.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    bufferDesc.ByteWidth = sizeof(PrimitiveJointConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    result = Graphics::GetInstance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, primitiveJointCbuffer_.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

void Model::Render(const DirectX::XMFLOAT4X4 world, const std::vector<Node>& animatedNodes)
{
    using namespace DirectX;

    Graphics& graphics = Graphics::GetInstance();

    const std::vector<Node>& nodes = animatedNodes.size() > 0 ? animatedNodes : Model::nodes_;

    graphics.GetDeviceContext()->PSSetShaderResources(0, 1, materialResourceView_.GetAddressOf());

    graphics.GetDeviceContext()->VSSetShader(vertexShader_.Get(), nullptr, 0);
    graphics.GetDeviceContext()->PSSetShader(pixelShader_.Get(), nullptr, 0);
    graphics.GetDeviceContext()->IASetInputLayout(inputLayout_.Get());
    graphics.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    std::function<void(int)> traverse = [&](int nodeIndex)->void {
        const Node& node = nodes_.at(nodeIndex);
        if (node.mesh_ > -1)
        {
            const Mesh& mesh = meshes_.at(node.mesh_);
            for (std::vector<Mesh::Primitive>::const_reference primitive : mesh.primitives_)
            {
                ID3D11Buffer* vertexBuffers[]{
                    primitive.vertexBufferViews_.at("POSITION").buffer_.Get(),
                    primitive.vertexBufferViews_.at("NORMAL").buffer_.Get(),
                    primitive.vertexBufferViews_.at("TANGENT").buffer_.Get(),
                    primitive.vertexBufferViews_.at("TEXCOORD_0").buffer_.Get(),
                    primitive.vertexBufferViews_.at("JOINTS_0").buffer_.Get(),
                    primitive.vertexBufferViews_.at("WEIGHTS_0").buffer_.Get(),
                };

                UINT strides[]{
                    static_cast<UINT>(primitive.vertexBufferViews_.at("POSITION").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("NORMAL").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("TANGENT").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("TEXCOORD_0").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("JOINTS_0").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("WEIGHTS_0").strideInBytes_),
                };

                UINT offsets[_countof(vertexBuffers)] = {};
                graphics.GetDeviceContext()->IASetVertexBuffers(0, _countof(vertexBuffers), vertexBuffers, strides, offsets);
                graphics.GetDeviceContext()->IASetIndexBuffer(primitive.indexBufferView_.buffer_.Get(), primitive.indexBufferView_.format_, 0);

                PrimitiveConstants primitiveData = {};
                primitiveData.material_ = primitive.material_;
                primitiveData.hasTangent_ = primitive.vertexBufferViews_.at("TANGENT").buffer_ != NULL;
                primitiveData.skin_ = node.skin_;
                XMStoreFloat4x4(&primitiveData.world_, XMLoadFloat4x4(&node.globalTransform_) * XMLoadFloat4x4(&world));
                graphics.GetDeviceContext()->UpdateSubresource(primitiveCbuffer_.Get(), 0, 0, &primitiveData, 0, 0);
                graphics.GetDeviceContext()->VSSetConstantBuffers(0, 1, primitiveCbuffer_.GetAddressOf());
                graphics.GetDeviceContext()->PSSetConstantBuffers(0, 1, primitiveCbuffer_.GetAddressOf());


                // テクスチャ
                const Material& material = materials_.at(primitive.material_);
                const int textureIndices[]
                {
                    material.data_.pbrMetallicRoughness_.basecolorTexture_.index_,
                    material.data_.pbrMetallicRoughness_.metallicRoughenssTexture_.index_,
                    material.data_.normalTexture_.index_,
                    material.data_.emissiveTexture_.index_,
                    material.data_.occlusionTexture_.index_,
                };
                ID3D11ShaderResourceView* nullShaderResourceView = {};
                std::vector<ID3D11ShaderResourceView*> shaderResourceViews(_countof(textureIndices));
                for (int textureIndex = 0; textureIndex < shaderResourceViews.size(); ++textureIndex)
                {
                    shaderResourceViews.at(textureIndex) = textureIndices[textureIndex] > -1 ?
                        textureResourceViews_.at(textures_.at(textureIndices[textureIndex]).source_).Get() :
                        nullShaderResourceView;
                }
                graphics.GetDeviceContext()->PSSetShaderResources(1, static_cast<UINT>(shaderResourceViews.size()), shaderResourceViews.data());

                if (node.skin_ > -1)
                {
                    const Skin& skin = skins_.at(node.skin_);
                    PrimitiveJointConstants primitiveJointData = {};
                    for (size_t jointIndex = 0; jointIndex < skin.joints_.size(); ++jointIndex)
                    {
                        XMStoreFloat4x4(&primitiveJointData.matrices[jointIndex],
                            XMLoadFloat4x4(&skin.inverseBindMatrices_.at(jointIndex)) *
                            XMLoadFloat4x4(&nodes.at(skin.joints_.at(jointIndex)).globalTransform_) *
                            XMMatrixInverse(NULL, XMLoadFloat4x4(&node.globalTransform_)));
                    }
                    graphics.GetDeviceContext()->UpdateSubresource(primitiveJointCbuffer_.Get(), 0, 0, &primitiveJointData, 0, 0);
                    graphics.GetDeviceContext()->VSSetConstantBuffers(2, 1, primitiveJointCbuffer_.GetAddressOf());

                }

                // 描画
                graphics.GetDeviceContext()->DrawIndexed(static_cast<UINT>(primitive.indexBufferView_.Count()), 0, 0);
            }

        }
            for (std::vector<int>::value_type childIndex : node.children_)
            {
                traverse(childIndex);
            }
        };

    for (std::vector<int>::value_type nodeIndex : scenes_.at(0).nodes_)
    {
        traverse(nodeIndex);
    }
}

void Model::Animate(size_t animationIndex, float time, std::vector<Node>& animatedNodes)
{
    using namespace std;
    using namespace DirectX;

    function<size_t(const vector<float>&, float, float&)> indexOf{
        [](const vector<float>& timelines, float time, float& interpolationFactor)->size_t {
            const size_t keyframeCount = timelines.size();
            if (time > timelines.at(keyframeCount - 1))
            {
                interpolationFactor = 1.0f;
                return keyframeCount - 2;
            }
            else if (time < timelines.at(0))
            {
                interpolationFactor = 0.0f;
                return 0;
            }
            size_t keyframeIndex = 0;
            for (size_t timeIndex = 1; timeIndex < keyframeCount; ++timeIndex)
            {
                if (time < timelines.at(timeIndex))
                {
                    keyframeIndex = max<size_t>(0LL, timeIndex - 1);
                    break;
                }
            }
            interpolationFactor = (time - timelines.at(keyframeIndex + 0)) /
                (timelines.at(keyframeIndex + 1) - timelines.at(keyframeIndex + 0));
            return keyframeIndex;
            } };

    if (animations_.size() > 0)
    {
        const Animation& animation = animations_.at(animationIndex);
        for (vector<Animation::Channel>::const_reference channel : animation.channels_)
        {
            const Animation::Sampler& sampler = animation.samplers_.at(channel.sampler_);
            const vector<float>& timeline = animation.timelines_.at(sampler.input_);
            if (timeline.size() == 0)
            {
                continue;
            }
            float interpolationFactor = {};
            size_t keyframeIndex = indexOf(timeline, time, interpolationFactor);
            if (channel.targetPath_ == "scale")
            {
                const vector<XMFLOAT3>& scales = animation.scales_.at(sampler.output_);
                XMStoreFloat3(&animatedNodes.at(channel.targetNode_).scale_,
                    XMVectorLerp(XMLoadFloat3(&scales.at(keyframeIndex + 0)),
                        XMLoadFloat3(&scales.at(keyframeIndex + 1)), interpolationFactor));
            }
            else if (channel.targetPath_ == "rotation")
            {
                const vector<XMFLOAT4>& rotations = animation.rotations_.at(sampler.output_);
                XMStoreFloat4(&animatedNodes.at(channel.targetNode_).rotation_,
                    XMQuaternionNormalize(XMQuaternionSlerp(XMLoadFloat4(&rotations.at(keyframeIndex + 0)),
                        XMLoadFloat4(&rotations.at(keyframeIndex + 1)), interpolationFactor)));
            }
            else if (channel.targetPath_ == "translation")
            {
                const vector<XMFLOAT3>& translations = animation.translations_.at(sampler.output_);
                XMStoreFloat3(&animatedNodes.at(channel.targetNode_).translation_,
                    XMVectorLerp(XMLoadFloat3(&translations.at(keyframeIndex + 0)),
                        XMLoadFloat3(&translations.at(keyframeIndex + 1)), interpolationFactor));
            }
        }
        CumulateTransforms(animatedNodes);
    }
}

#if 1
// Node情報
void Model::FetchNodes(const tinygltf::Model& gltfModel)
{
    for (std::vector<tinygltf::Node>::const_reference gltfNode : gltfModel.nodes)
    {
        Node& node = nodes_.emplace_back();
        node.name_ = gltfNode.name;
        node.skin_ = gltfNode.skin;
        node.mesh_ = gltfNode.mesh;
        node.children_ = gltfNode.children;

        if (!gltfNode.matrix.empty())
        {
            DirectX::XMFLOAT4X4 matrix;
            for (size_t row = 0; row < 4; row++)
            {
                for (size_t column = 0; column < 4; column++)
                {
                    matrix(row, column) = static_cast<float>(gltfNode.matrix.at(4 * row + column));
                }
            }

            DirectX::XMVECTOR S, R, T;
            bool succeeded = DirectX::XMMatrixDecompose(&S, &R, &T, DirectX::XMLoadFloat4x4(&matrix));
            _ASSERT_EXPR(succeeded, L"Faild to decompose matrix.");

            DirectX::XMStoreFloat3(&node.scale_, S);
            DirectX::XMStoreFloat4(&node.rotation_, R);
            DirectX::XMStoreFloat3(&node.translation_, T);
        }
        else
        {
            if (gltfNode.scale.size() > 0)
            {
                node.scale_.x = static_cast<float>(gltfNode.scale.at(0));
                node.scale_.y = static_cast<float>(gltfNode.scale.at(1));
                node.scale_.z = static_cast<float>(gltfNode.scale.at(2));
            }
            if (gltfNode.rotation.size() > 0)
            {
                node.rotation_.x = static_cast<float>(gltfNode.rotation.at(0));
                node.rotation_.y = static_cast<float>(gltfNode.rotation.at(1));
                node.rotation_.z = static_cast<float>(gltfNode.rotation.at(2));
                node.rotation_.w = static_cast<float>(gltfNode.rotation.at(3));
            }
            if (gltfNode.translation.size() > 0)
            {
                node.translation_.x = static_cast<float>(gltfNode.translation.at(0));
                node.translation_.y = static_cast<float>(gltfNode.translation.at(1));
                node.translation_.z = static_cast<float>(gltfNode.translation.at(2));
            }
        }
    }
    CumulateTransforms(nodes_);
}
#endif

// Mesh情報
void Model::FetchMeshes(const tinygltf::Model& gltfModel)
{
    HRESULT result;

    for (std::vector<tinygltf::Mesh>::const_reference gltfMesh : gltfModel.meshes)
    {
        Mesh& mesh = meshes_.emplace_back();
        mesh.name_ = gltfMesh.name;
        for (std::vector<tinygltf::Primitive>::const_reference gltfPrimitive : gltfMesh.primitives)
        {
            Mesh::Primitive& primitive = mesh.primitives_.emplace_back();
            primitive.material_ = gltfPrimitive.material;

            // インデックスバッファの作成
            const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(gltfPrimitive.indices);
            const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);

            primitive.indexBufferView_ = MakeBufferView(gltfAccessor);

            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.ByteWidth = static_cast<UINT>(primitive.indexBufferView_.sizeInBytes_);
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            D3D11_SUBRESOURCE_DATA subresourceData = {};
            subresourceData.pSysMem = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;

            result = Graphics::GetInstance().GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, primitive.indexBufferView_.buffer_.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

            // 頂点バッファの作成
            for (std::map<std::string, int>::const_reference gltfAttribute : gltfPrimitive.attributes)
            {
                const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(gltfAttribute.second);
                const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);

                BufferView vertexBufferView = MakeBufferView(gltfAccessor);

                D3D11_BUFFER_DESC bufferDesc = {};
                bufferDesc.ByteWidth = static_cast<UINT>(vertexBufferView.sizeInBytes_);
                bufferDesc.Usage = D3D11_USAGE_DEFAULT;
                bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                D3D11_SUBRESOURCE_DATA subresourceData = {};
                subresourceData.pSysMem = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;

                result = Graphics::GetInstance().GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, vertexBufferView.buffer_.ReleaseAndGetAddressOf());
                _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

                primitive.vertexBufferViews_.emplace(std::make_pair(gltfAttribute.first, vertexBufferView));
            }

            // 不足している場合はダミーの属性を追加する
            const std::unordered_map<std::string, BufferView> attributes{
                { "TANGENT",    {DXGI_FORMAT_R32G32B32A32_FLOAT } },
                { "TEXCOORD_0", {DXGI_FORMAT_R32G32_FLOAT} },
                { "JOINTS_0",   {DXGI_FORMAT_R16G16B16A16_UINT} },
                { "WEIGHTS_0",  {DXGI_FORMAT_R32G32B32A32_FLOAT} },
            };

            for (std::unordered_map<std::string, BufferView>::const_reference attribute : attributes)
            {
                if (primitive.vertexBufferViews_.find(attribute.first) == primitive.vertexBufferViews_.end())
                {
                    primitive.vertexBufferViews_.insert(std::make_pair(attribute.first, attribute.second));
                }
            }
        }
    }
}

// Material情報
void Model::FetchMaterials(const tinygltf::Model& gltfModel)
{
    for (std::vector<tinygltf::Material>::const_reference gltfMaterial : gltfModel.materials)
    {
        std::vector<Material>::reference material = materials_.emplace_back();

        material.name_ = gltfMaterial.name;

        material.data_.emissiveFactor_[0] = static_cast<float>(gltfMaterial.emissiveFactor.at(0));
        material.data_.emissiveFactor_[1] = static_cast<float>(gltfMaterial.emissiveFactor.at(1));
        material.data_.emissiveFactor_[2] = static_cast<float>(gltfMaterial.emissiveFactor.at(2));

        material.data_.alphaMode_ = gltfMaterial.alphaMode == "OPAQUE" ? 0 : gltfMaterial.alphaMode == "MASK" ? 1 : gltfMaterial.alphaMode == "BLEND" ? 2 : 0;
        material.data_.alphaCutoff_ = static_cast<float>(gltfMaterial.alphaCutoff);
        material.data_.doubleSided_ = gltfMaterial.doubleSided ? 1 : 0;

        material.data_.pbrMetallicRoughness_.basecolorFactor_[0] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(0));
        material.data_.pbrMetallicRoughness_.basecolorFactor_[1] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(1));
        material.data_.pbrMetallicRoughness_.basecolorFactor_[2] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(2));
        material.data_.pbrMetallicRoughness_.basecolorFactor_[3] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(3));
        material.data_.pbrMetallicRoughness_.basecolorTexture_.index_ = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
        material.data_.pbrMetallicRoughness_.basecolorTexture_.texcoord_ = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
        material.data_.pbrMetallicRoughness_.metallicFactor_ = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
        material.data_.pbrMetallicRoughness_.roughnessFactor_ = static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);
        material.data_.pbrMetallicRoughness_.metallicRoughenssTexture_.index_ = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
        material.data_.pbrMetallicRoughness_.metallicRoughenssTexture_.texcoord_ = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;

        material.data_.normalTexture_.index_ = gltfMaterial.normalTexture.index;
        material.data_.normalTexture_.texcoord_ = gltfMaterial.normalTexture.texCoord;
        material.data_.normalTexture_.scale_ = static_cast<float>(gltfMaterial.normalTexture.scale);

        material.data_.occlusionTexture_.index_ = gltfMaterial.occlusionTexture.index;
        material.data_.occlusionTexture_.texcoord_ = gltfMaterial.occlusionTexture.texCoord;
        material.data_.occlusionTexture_.strength_ = static_cast<float>(gltfMaterial.occlusionTexture.strength);

        material.data_.emissiveTexture_.index_ = gltfMaterial.emissiveTexture.index;
        material.data_.emissiveTexture_.texcoord_ = gltfMaterial.emissiveTexture.texCoord;
    }

    // GPU上でシェーダーリソースビューとしてマテリアルデータを作成
    std::vector<Material::Cbuffer> materialData;
    for (std::vector<Material>::const_reference material : materials_)
    {
        materialData.emplace_back(material.data_);
    }

    HRESULT result;
    Microsoft::WRL::ComPtr<ID3D11Buffer> materialBuffer;
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Material::Cbuffer) * materialData.size());
    bufferDesc.StructureByteStride = sizeof(Material::Cbuffer);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    D3D11_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pSysMem = materialData.data();

    result = Graphics::GetInstance().GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, materialBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    shaderResourceViewDesc.Buffer.NumElements = static_cast<UINT>(materialData.size());

    result = Graphics::GetInstance().GetDevice()->CreateShaderResourceView(materialBuffer.Get(), &shaderResourceViewDesc, materialResourceView_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

}

// Texture情報
void Model::FetchTexutres(const tinygltf::Model& gltfModel)
{
    HRESULT result = S_OK;

    for (const tinygltf::Texture& gltfTexture : gltfModel.textures)
    {
        Texture& texture = textures_.emplace_back();
        texture.name_ = gltfTexture.name;
        texture.source_ = gltfTexture.source;
    }
    for (const tinygltf::Image& gltfImage : gltfModel.images)
    {
        Image& image = images_.emplace_back();
        image.name_ = gltfImage.name;
        image.width_ = gltfImage.width;
        image.height_ = gltfImage.height;
        image.component_ = gltfImage.component;
        image.bits_ = gltfImage.bits;
        image.pixelType_ = gltfImage.pixel_type;
        image.bufferView_ = gltfImage.bufferView;
        image.mimeType_ = gltfImage.mimeType;
        image.uri_ = gltfImage.uri;
        image.asIs_ = gltfImage.as_is;

        if (gltfImage.bufferView > -1)
        {
            const tinygltf::BufferView& bufferView = gltfModel.bufferViews.at(gltfImage.bufferView);
            const tinygltf::Buffer& buffer = gltfModel.buffers.at(bufferView.buffer);
            const unsigned char* data = buffer.data.data() + bufferView.byteOffset;

            ID3D11ShaderResourceView* textureResourceView = {};
            result = LoadTextureFromMemory(Graphics::GetInstance().GetDevice(), data, bufferView.byteLength, &textureResourceView);
            if (result == S_OK)
            {
                textureResourceViews_.emplace_back().Attach(textureResourceView);
            }
        }
        else
        {
            const std::filesystem::path path(filename_);
            ID3D11ShaderResourceView* shaderResourceView = {};
            D3D11_TEXTURE2D_DESC texture2dDesc;
            std::wstring filename = path.parent_path().concat(L"/").wstring() + std::wstring(gltfImage.uri.begin(), gltfImage.uri.end());

            result = LoadTextureFromFile(Graphics::GetInstance().GetDevice(), filename.c_str(), &shaderResourceView, &texture2dDesc);
            if (result == S_OK)
            {
                textureResourceViews_.emplace_back().Attach(shaderResourceView);
            }
        }
    }
}

void Model::FetchAnimations(const tinygltf::Model& gltfModel)
{
    using namespace std;
    using namespace tinygltf;
    using namespace DirectX;

    for (vector<tinygltf::Skin>::const_reference transmissionSkin : gltfModel.skins)
    {
        Skin& skin = skins_.emplace_back();
        const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(transmissionSkin.inverseBindMatrices);
        const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);
        skin.inverseBindMatrices_.resize(gltfAccessor.count);
        memcpy(skin.inverseBindMatrices_.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() +
            gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(XMFLOAT4X4));
        skin.joints_ = transmissionSkin.joints;
    }

    for (vector<tinygltf::Animation>::const_reference gltfAnimation : gltfModel.animations)
    {
        Animation& animation = animations_.emplace_back();
        animation.name_ = gltfAnimation.name;
        for (vector<AnimationSampler>::const_reference gltfSampler : gltfAnimation.samplers)
        {
            Animation::Sampler& sampler = animation.samplers_.emplace_back();
            sampler.input_ = gltfSampler.input;
            sampler.output_ = gltfSampler.output;
            sampler.interpolation_ = gltfSampler.interpolation;

            const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(gltfSampler.input);
            const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);
            const pair<unordered_map<int, vector<float>>::iterator, bool>& timelines = 
                animation.timelines_.emplace(gltfSampler.input, gltfAccessor.count);
            if (timelines.second)
            {
                memcpy(timelines.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() +
                    gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(FLOAT));
            }
        }

        for (vector<tinygltf::AnimationChannel>::const_reference gltfChannel : gltfAnimation.channels)
        {
            Animation::Channel& channel = animation.channels_.emplace_back();
            channel.sampler_ = gltfChannel.sampler;
            channel.targetNode_ = gltfChannel.target_node;
            channel.targetPath_ = gltfChannel.target_path;

            const tinygltf::AnimationSampler& gltfSampler = gltfAnimation.samplers.at(gltfChannel.sampler);
            const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(gltfSampler.output);
            const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);
            if (gltfChannel.target_path == "scale")
            {
                const pair<unordered_map<int, vector<XMFLOAT3>>::iterator, bool>& scales = 
                    animation.scales_.emplace(gltfSampler.output, gltfAccessor.count);
                if (scales.second)
                {
                    memcpy(scales.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() +
                        gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(XMFLOAT3));
                }
            }
            else if (gltfChannel.target_path == "rotation")
            {
                const pair<unordered_map<int, vector<XMFLOAT4>>::iterator, bool>& rotations = 
                    animation.rotations_.emplace(gltfSampler.output, gltfAccessor.count);
                if (rotations.second)
                {
                    memcpy(rotations.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() +
                        gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(XMFLOAT4));
                }
            }
            else if (gltfChannel.target_path == "translation")
            {
                const pair<unordered_map<int, vector<XMFLOAT3>>::iterator, bool>& translations =
                    animation.translations_.emplace(gltfSampler.output, gltfAccessor.count);
                if (translations.second)
                {
                    memcpy(translations.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() +
                        gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(XMFLOAT3));
                }
            }
        }
    }
    // 各チャネルのタイムラインで最長のアニメーションの長さを見つける。
    for (decltype(animations_)::reference animation : animations_)
    {
        for (decltype(animation.timelines_)::reference timelines : animation.timelines_)
        {
            animation.duration_ = std::max<float>(animation.duration_, timelines.second.back());
        }
    }
}

void Model::CumulateTransforms(std::vector<Node>& nodes)
{
    using namespace DirectX;

    std::stack<XMFLOAT4X4> parentGlobalTransforms;
    std::function<void(int)> traverse{ [&](int nodeIndex)->void
    {
        Node& node = nodes.at(nodeIndex);
        XMMATRIX S = XMMatrixScaling(node.scale_.x, node.scale_.y, node.scale_.z);
        XMMATRIX R = XMMatrixRotationQuaternion(XMVectorSet(node.rotation_.x, node.rotation_.y, node.rotation_.z, node.rotation_.w));
        XMMATRIX T = XMMatrixTranslation(node.translation_.x, node.translation_.y, node.translation_.z);
        XMStoreFloat4x4(&node.globalTransform_, S * R * T * XMLoadFloat4x4(&parentGlobalTransforms.top()));
        for (int childIndex : node.children_)
        {
            parentGlobalTransforms.push(node.globalTransform_);
            traverse(childIndex);
            parentGlobalTransforms.pop();
        }
    } };

    for (std::vector<int>::value_type nodeIndex : scenes_.at(0).nodes_)
    {
        parentGlobalTransforms.push({ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 });
        traverse(nodeIndex);
        parentGlobalTransforms.pop();
    }
}

Model::BufferView Model::MakeBufferView(const tinygltf::Accessor& accessor)
{
    BufferView bufferView;

    switch (accessor.type)
    {
    case TINYGLTF_TYPE_SCALAR:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            bufferView.format_ = DXGI_FORMAT_R16_UINT;
            bufferView.strideInBytes_ = sizeof(USHORT);
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            bufferView.format_ = DXGI_FORMAT_R32_UINT;
            bufferView.strideInBytes_ = sizeof(UINT);
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            break;
        } // end of accessor.componentType
        break;

    case TINYGLTF_TYPE_VEC2:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            bufferView.format_ = DXGI_FORMAT_R32G32_FLOAT;
            bufferView.strideInBytes_ = sizeof(FLOAT) * 2;
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            break;
        } // end of accessor.componentType
        break;

    case TINYGLTF_TYPE_VEC3:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            bufferView.format_ = DXGI_FORMAT_R32G32B32_FLOAT;
            bufferView.strideInBytes_ = sizeof(FLOAT) * 3;
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            break;
        } // end of accessor.componentType
        break;

    case TINYGLTF_TYPE_VEC4:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            bufferView.format_ = DXGI_FORMAT_R8G8B8A8_UINT;
            bufferView.strideInBytes_ = sizeof(BYTE) * 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            bufferView.format_ = DXGI_FORMAT_R16G16B16A16_UINT;
            bufferView.strideInBytes_ = sizeof(USHORT) * 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            bufferView.format_ = DXGI_FORMAT_R32G32B32A32_UINT;
            bufferView.strideInBytes_ = sizeof(UINT) * 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            bufferView.format_ = DXGI_FORMAT_R32G32B32A32_FLOAT;
            bufferView.strideInBytes_ = sizeof(FLOAT) * 4;
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            break;
        } // end of accessor.componentType
        break;

    default:
        _ASSERT_EXPR(FALSE, L"This accessor type is not supported.");
        break;
    } // end of accessor type

    bufferView.sizeInBytes_ = static_cast<UINT>(accessor.count * bufferView.strideInBytes_);

    return bufferView;
}
