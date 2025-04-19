#pragma once
#include "Graphics.h"

class BaseScene
{
public:
    BaseScene() {}
    virtual ~BaseScene() {}

    virtual void Initialize()               = 0; // 初期化
    virtual void Finalize()                 = 0; // 終了化
    virtual void Update(float elapsedTime)  = 0; // 更新処理
    virtual void Render()                   = 0; // 描画処理
    virtual void DrawDebug()                = 0; // デバッグ

    // 基本的な2D用描画ステートを設定
    void Set2DRendeing()
    {
        Graphics& graphics = Graphics::GetInstance();

        graphics.SetDepthStencilState(Shader::DepthState::NoTestNoWirte);
        graphics.SetBlendState(Shader::BlendState::Alpha);
        graphics.SetRasterizerState(Shader::RasterState::CullNone);
    }

    // 基本的な3D用描画ステートを設定
    void Set3DRendering()
    {
        Graphics& graphics = Graphics::GetInstance();

        graphics.SetDepthStencilState(Shader::DepthState::TestAndWrite);
        graphics.SetBlendState(Shader::BlendState::None);
        graphics.SetRasterizerState(Shader::RasterState::CullNone);
    }
};