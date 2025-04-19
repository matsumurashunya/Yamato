#pragma once
#include <d3d11.h>
#include <wrl.h>

class Shader
{
public:
    // ブレンドステート
    enum class BlendState
    {
        None,
        Alpha,
        Add,
        Multiply,

        EnumCount
    };

    // デプスステート
    enum class DepthState
    {
        TestAndWrite,
        Test,
        Write,
        NoTestNoWirte,

        EnumCount
    };

    // サンプラーステート
    enum class SamplerState
    {
        Point,
        Linaer,
        Anisotropic,
        LinearBorderBlack,
        LinearBorderWhite,
        Comparison,

        EnumCount
    };

    // ラスタライザーステート
    enum class RasterState
    {
        Solid,
        Wireframe,
        CullNone,
        WireframeCullNone,

        EnumCount
    };

public:
    Shader();
    ~Shader() {}

    // シェーダーオブジェクト生成
    HRESULT CreateVsFromCso(const char* csoName, ID3D11VertexShader** vertexShader, ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC* inputElementDesc, UINT numElements);
    HRESULT CreatePsFromCso(const char* csoName, ID3D11PixelShader** pixelShader);

    // 描画設定
    void SetBlendState(const BlendState& blendState);
    void SetRasterizerState(const RasterState& rasterState);
    void SetDepthStencilState(const DepthState& depthState);
    void SetSamplerState();

private:
    // 各ステート作成
    void CreateBlendStates();           // ブレンドステート作成
    void CreateDepthStencilStates();    // デプスステンシルステート作成
    void CreateSamplerStates();         // サンプラーステート作成
    void CreateRasterizerStates();      // ラスタライザーステート作成

private:
    Microsoft::WRL::ComPtr<ID3D11BlendState>        blendStates_[4];
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStates_[static_cast<int>(DepthState::EnumCount)];
    Microsoft::WRL::ComPtr<ID3D11SamplerState>      samplerStates_[4];
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   rasterizerStates_[4];
};

