#include "Sprite.h"
#include "Graphics.h"
#include "Misc.h"

// コンストラクタ
Sprite::Sprite(const wchar_t* filename)
{
    // 頂点情報のセット
    Vertex vertices[]
    {
        { { -1.0, +1.0, 0 }, { 1, 1, 1, 1 }, { 0, 0 } },
        { { +1.0, +1.0, 0 }, { 1, 1, 1, 1 }, { 1, 0 } },
        { { -1.0, -1.0, 0 }, { 1, 1, 1, 1 }, { 0, 1 } },
        { { +1.0, -1.0, 0 }, { 1, 1, 1, 1 }, { 1, 1 } },
    };

    // 頂点バッファオブジェクトの生成
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth            = sizeof(vertices);             // バッファのサイズ（バイト）
    bufferDesc.Usage                = D3D11_USAGE_DYNAMIC;          // バッファの使用方法
    bufferDesc.BindFlags            = D3D11_BIND_VERTEX_BUFFER;     // バッファのバインド（頂点バッファ）
    bufferDesc.CPUAccessFlags       = D3D10_CPU_ACCESS_WRITE;       // CPUからのアクセス方法（アクセスなし）
    bufferDesc.MiscFlags            = 0;                            // バッファの追加的なフラグを指定（特になし）
    bufferDesc.StructureByteStride  = 0;

    // 頂点シェーダーオブジェクトの生成
    D3D11_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pSysMem             = vertices;              // GPUメモリにコピーされるデータへのポインタ（頂点データ）
    subresourceData.SysMemPitch         = 0;                     // 一行あたりのバッファのメモリバイト数（特になし）
    subresourceData.SysMemSlicePitch    = 0;                     // テクスチャデータのスライスのピッチ（特になし）

    HRESULT result = S_OK;

    result = Graphics::GetInstance().GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, vertexBuffer_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    // 入力レイアウトオブジェクト
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",      0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // シェーダー読み込み
    Graphics::GetInstance().GetShader()->CreateVsFromCso("./Resources/Shader/SpriteVS.cso", vertexShader_.GetAddressOf(), inputLayout_.GetAddressOf(), inputElementDesc, _countof(inputElementDesc));
    Graphics::GetInstance().GetShader()->CreatePsFromCso("./Resources/Shader/SpritePS.cso", pixelShader_.GetAddressOf());

    // テクスチャ情報
}

// 描画処理
void Sprite::Render()
{
}

// 描画処理指定あり
void Sprite::Render(float posX, float posY, float sizeX, float sizeY,
    float texX, float texY, float texW, float texH, float pivotX,
    float pivotY, float angle, float r, float g, float b, float a)
{

}

// 回転処理
void Sprite::Rotate(float& x, float& y, const float& centerX, const float& centerY, const float& angle)
{
    // 指定の位置に動かす
    x -= centerX;
    y -= centerY;

    // 回転処理
    float cos = cosf(DirectX::XMConvertToRadians(angle));
    float sin = sinf(DirectX::XMConvertToRadians(angle));
    float tx = x;
    float ty = y;
    x = cos * tx + -sin * ty;
    y = sin * tx + cos * ty;

    // 元の位置に戻す
    x += centerX;
    y += centerY;
}

// デバッグ
void Sprite::DrawDebug()
{
}
