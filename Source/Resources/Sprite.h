#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>

class Sprite
{
public:
    Sprite(const wchar_t* filename);
    ~Sprite() {}

    // 描画処理
    void Render();

    // 描画処理
    // posX, posY       : 描画位置
    // sizeX, sizeY     : 描画サイズ
    // texX, texY       : 元画像の位置
    // texW, texH       : 元画像の幅、高さ
    // pivotX, pivotY   : 基準点
    // angle            : 角度
    // r, g, b, a       : 色, 透明度
    void Render(float posX, float posY,
        float sizeX, float sizeY,
        float texX, float texY,
        float texW, float texH,
        float pivotX, float pivotY,
        float angle,
        float r, float g, float b, float a = 1.0f);

    // デバッグ
    void DrawDebug();

private:
    struct Vertex // 頂点情報
    {
        DirectX::XMFLOAT3 position_;
        DirectX::XMFLOAT4 color_;
        DirectX::XMFLOAT2 texcoord_;
    };

private:
    // 回転処理
    void Rotate(float& x, float& y, const float& centerX, const float& centerY, const float& angle);

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader>          vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>           pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>           inputLayout_;
    Microsoft::WRL::ComPtr<ID3D11Buffer>                vertexBuffer_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    shaderResourceView_;
};

