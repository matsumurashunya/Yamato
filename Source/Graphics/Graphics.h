#pragma once
#include <d3d11_1.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <memory>
#include "Shader/Shader.h"

// TODO 仮
#include "FullscreenQuad.h"
#include "FrameBuffer.h"

CONST LONG SCREEN_WIDTH = 1280;
CONST LONG SCREEN_HEIGHT = 720;
CONST LPCWSTR APPLICATION_NAME = L"Yamato";
CONST BOOL FULLSCREEN = FALSE;

class Graphics
{
private:
    Graphics() {}
    ~Graphics() {}

public:
    static Graphics& GetInstance()
    {
        static Graphics instance;
        return instance;
    }

    // 初期化
    void Initialize(HWND hWnd);

    // 描画初期化
    void Clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);

    // 描画
    void Present();

    // TODO fullscreen
    // ウィンドウサイズ変更
    void OnResize(UINT64 width, UINT heigth, HWND hWnd);
    void acquireHighPerformanceAdapter(IDXGIFactory6* dxgiFactory6, IDXGIAdapter3** dxgiAdapter3);
    size_t videoMemoryUsage();
    void StylizeWndiow(HWND hWnd, BOOL fullscreen = FALSE);
    //void CreateSwapChain(IDXGIFactory6* dxgiFactory6);
    void CreateSwapChain(IDXGIFactory6* dxgiFactory6, HWND hWnd);
    //void RecreateSwapChain();

    HRESULT CreateVsFromCso(const char* csoName, ID3D11VertexShader** vertexShader, ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC* inputElementDesc, UINT numElements) { return shader_->CreateVsFromCso(csoName, vertexShader, inputLayout, inputElementDesc, numElements); }
    HRESULT CreatePsFromCso(const char* csoName, ID3D11PixelShader** pixelShader) { return shader_->CreatePsFromCso(csoName, pixelShader); }

    // ゲッター
    ID3D11Device*           GetDevice()             const { return device_.Get(); }
    ID3D11DeviceContext*    GetDeviceContext()      const { return deviceContext_.Get(); }
    IDXGISwapChain*         GetSwapChain()          const { return swapChain_.Get(); }
    ID3D11RenderTargetView* GetRenderTargetView()   const { return renderTargetView_.Get(); }
    ID3D11DepthStencilView* GetDepthStencilView()   const { return depthStencilView_.Get(); }
    // シェーダー
    Shader* GetShader() { return shader_.get(); }

    BOOL GetFullscreen() { return fullscreenMode_; }

    // セッター
    void SetDepthStencilState(const Shader::DepthState& depthStencilState) { shader_->SetDepthStencilState(depthStencilState); }
    void SetBlendState(const Shader::BlendState& blendState) { shader_->SetBlendState(blendState); }
    void SetRasterizerState(const Shader::RasterState& rasterizerSate) { shader_->SetRasterizerState(rasterizerSate); }

    //TODO 仮
    //{
    void Screen()
    {
        ID3D11ShaderResourceView* shaderResourceViews[2] = { frameBuffer_[0]->shaderResourceViews_[0].Get(), frameBuffer_[1]->shaderResourceViews_[0].Get() };
        bitBlockTransfer_->Blit(deviceContext_.Get(), shaderResourceViews, 0, 2, pixelShaders_[1].Get());
    }

    std::unique_ptr<FrameBuffer> frameBuffer_[8];
    std::unique_ptr<FullscreenQuad> bitBlockTransfer_;;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaders_[8];
    //}


private:
    // ビューポート作成
    void CreateViewport();

    // レンダーターゲットビュー作成
    void CreateRenderTargetView();

    // デプスステンシルビュー作成
    void CreateDepthStencilView();

    // 描画設定再設定
    void RecreateRenderTargets();

private:
    CONST HWND hWnd_ = nullptr;
    std::unique_ptr<Shader> shader_; // レンダー設定

    Microsoft::WRL::ComPtr<ID3D11Device>            device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>     deviceContext_;
    //Microsoft::WRL::ComPtr<IDXGISwapChain>          swapChain_;
    Microsoft::WRL::ComPtr<IDXGISwapChain1>          swapChain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  renderTargetView_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>         depthStencilBuffer_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  depthStencilView_;

    // TODO fullscreen
    Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter_;
    Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory6_;
    SIZE framebufferDimensions_ = {};
    BOOL fullscreenMode_ = FALSE;
    BOOL tearingSupported_ = FALSE;
    RECT windowedRect_ = {};

    UINT syncInterval_ = 1; // 垂直同期間隔設定
};