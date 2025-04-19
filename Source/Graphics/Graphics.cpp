#include "Graphics.h"
#include "Misc.h"
#include <string>

// 初期化
void Graphics::Initialize(HWND hWnd)
{
    HRESULT result          = S_OK;

    GetWindowRect(hWnd, &windowedRect_);
    StylizeWndiow(hWnd, fullscreenMode_);

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    framebufferDimensions_.cx = clientRect.right - clientRect.left;
    framebufferDimensions_.cy = clientRect.bottom - clientRect.top;

    UINT createFactoryFlags = 0;
#ifdef _DEBUG
    createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    result = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(dxgiFactory6_.GetAddressOf()));
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    acquireHighPerformanceAdapter(dxgiFactory6_.Get(), adapter_.GetAddressOf());

    UINT createDeviceFlags  = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    // DirectXのバージョン
    D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_1;
    result = D3D11CreateDevice(adapter_.Get(), D3D_DRIVER_TYPE_UNKNOWN, 0, createDeviceFlags,
        &featureLevels, 1, D3D11_SDK_VERSION, &device_, NULL, &deviceContext_);
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    CreateSwapChain(dxgiFactory6_.Get(), hWnd);

//    // スワップチェーンの設定
//    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
//    swapChainDesc.BufferCount                           = 1;
//    swapChainDesc.BufferDesc.Width                      = SCREEN_WIDTH;
//    swapChainDesc.BufferDesc.Height                     = SCREEN_HEIGHT;
//    swapChainDesc.BufferDesc.Format                     = DXGI_FORMAT_R8G8B8A8_UNORM;
//    swapChainDesc.BufferDesc.RefreshRate.Numerator      = 60;
//    swapChainDesc.BufferDesc.RefreshRate.Denominator    = 1;
//    swapChainDesc.BufferUsage                           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//    swapChainDesc.OutputWindow                          = hWnd;
//    swapChainDesc.SampleDesc.Count                      = 1;
//    swapChainDesc.SampleDesc.Quality                    = 0;
//#ifdef _DEBUG
//    swapChainDesc.Windowed                              = !fullscreenMode_;
//#else
//    swapChainDesc.Windowed                              = fullscreenMode_;
//#endif
//
//    // デバイス、スワップチェーンの生成
//    result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
//        createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc,
//        swapChain_.GetAddressOf(), device_.GetAddressOf(), NULL, deviceContext_.GetAddressOf());
//    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    // レンダーターゲットビュー作成
    //CreateRenderTargetView();

    // デプスステンシルビュー作成
    //CreateDepthStencilView();

    //// ビューポート作成
    //D3D11_VIEWPORT viewport = {};
    //viewport.TopLeftX   = 0.0;
    //viewport.TopLeftY   = 0.0;
    //viewport.Width      = static_cast<float>(SCREEN_WIDTH);
    //viewport.Height     = static_cast<float>(SCREEN_HEIGHT);
    //viewport.MinDepth   = 0.0;
    //viewport.MaxDepth   = 1.0;
    //deviceContext_->RSSetViewports(1, &viewport);

    //CreateViewport();

    // 描画設定などの設定
    shader_ = std::make_unique<Shader>();

    // フレームバッファ生成
    frameBuffer_[0] = std::make_unique<FrameBuffer>(device_.Get(), SCREEN_WIDTH, SCREEN_HEIGHT);
    frameBuffer_[1] = std::make_unique<FrameBuffer>(device_.Get(), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    // fullscreenQuad生成
    bitBlockTransfer_ = std::make_unique<FullscreenQuad>(device_.Get());
    CreatePsFromCso("./Resources/Shader/LuminanceExtraction.cso", pixelShaders_[0].GetAddressOf());
    CreatePsFromCso("./Resources/Shader/BlurPS.cso", pixelShaders_[1].GetAddressOf());
}

// 描画初期化
void Graphics::Clear(float r, float g, float b, float a)
{
    FLOAT color[4] = { r, g, b, a };
    deviceContext_->ClearRenderTargetView(renderTargetView_.Get(), color);
    //deviceContext_->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // レンダーターゲット設定
    // GPUに初期化した deviceContext_ の情報を送ります
    deviceContext_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());
}

// 描画
void Graphics::Present()
{
    swapChain_->Present(syncInterval_, 0);
}

// ウィンドウサイズ変更
void Graphics::OnResize(UINT64 width, UINT height, HWND hWnd)
{
    // フレームバッファのサイズの更新
    if (width > 0 && height > 0 && (width != framebufferDimensions_.cx || height != framebufferDimensions_.cy))
    {
        framebufferDimensions_.cx = static_cast<LONG>(width);
        framebufferDimensions_.cy = height;

        // 更新された寸法でフレームバッファを再作成
        frameBuffer_[0] = std::make_unique<FrameBuffer>(device_.Get(), framebufferDimensions_.cx, framebufferDimensions_.cy);
        frameBuffer_[1] = std::make_unique<FrameBuffer>(device_.Get(), framebufferDimensions_.cx / 2, framebufferDimensions_.cy / 2);




        // サイズ変更する前に、すべての以前のGPUコマンドが終了していることを確認
        deviceContext_->Flush();
        deviceContext_->ClearState();

        // スワップチェーン再作成
        CreateSwapChain(dxgiFactory6_.Get(),hWnd);
        //RecreateRenderTargets();
        //CreateViewport();
    }
}

void Graphics::acquireHighPerformanceAdapter(IDXGIFactory6* dxgiFactory6, IDXGIAdapter3** dxgiAdapter3)
{
    HRESULT result = S_OK;

    Microsoft::WRL::ComPtr<IDXGIAdapter3> enumerated_adapter;
    for (UINT adapter_index = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory6->EnumAdapterByGpuPreference(adapter_index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(enumerated_adapter.ReleaseAndGetAddressOf())); ++adapter_index)
    {
        DXGI_ADAPTER_DESC1 adapter_desc;
        result = enumerated_adapter->GetDesc1(&adapter_desc);
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

        if (adapter_desc.VendorId == 0x1002/*AMD*/ || adapter_desc.VendorId == 0x10DE/*NVIDIA*/)
        {
            OutputDebugStringW((std::wstring(adapter_desc.Description) + L" has been selected.\n").c_str());
            OutputDebugStringA(std::string("\tVendorId:" + std::to_string(adapter_desc.VendorId) + '\n').c_str());
            OutputDebugStringA(std::string("\tDeviceId:" + std::to_string(adapter_desc.DeviceId) + '\n').c_str());
            OutputDebugStringA(std::string("\tSubSysId:" + std::to_string(adapter_desc.SubSysId) + '\n').c_str());
            OutputDebugStringA(std::string("\tRevision:" + std::to_string(adapter_desc.Revision) + '\n').c_str());
            OutputDebugStringA(std::string("\tDedicatedVideoMemory:" + std::to_string(adapter_desc.DedicatedVideoMemory) + '\n').c_str());
            OutputDebugStringA(std::string("\tDedicatedSystemMemory:" + std::to_string(adapter_desc.DedicatedSystemMemory) + '\n').c_str());
            OutputDebugStringA(std::string("\tSharedSystemMemory:" + std::to_string(adapter_desc.SharedSystemMemory) + '\n').c_str());
            OutputDebugStringA(std::string("\tAdapterLuid.HighPart:" + std::to_string(adapter_desc.AdapterLuid.HighPart) + '\n').c_str());
            OutputDebugStringA(std::string("\tAdapterLuid.LowPart:" + std::to_string(adapter_desc.AdapterLuid.LowPart) + '\n').c_str());
            OutputDebugStringA(std::string("\tFlags:" + std::to_string(adapter_desc.Flags) + '\n').c_str());
            break;
        }
    }
    *dxgiAdapter3 = enumerated_adapter.Detach();
}

size_t Graphics::videoMemoryUsage()
{
    DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
    adapter_->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);
    return videoMemoryInfo.CurrentUsage / 1024 / 1024;
}

// ウィンドウモード切り替え
void Graphics::StylizeWndiow(HWND hWnd, BOOL fullscreen)
{
    fullscreenMode_ = fullscreen;

    // フルスクリーンモード
    if (fullscreen)
    {
        // ウィンドウモードでウィンドウの位置を記録する
        GetWindowRect(hWnd, &windowedRect_);

        // 全画面用にウィンドウスタイルを変更する（タイトルバーとボーダーを削除する）
        DWORD fullscreenStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME);
        SetWindowLongPtrA(hWnd, GWL_STYLE, fullscreenStyle);

        RECT fullscreenWindowRect;

        HRESULT result = E_FAIL;
        if (swapChain_)
        {
            Microsoft::WRL::ComPtr<IDXGIOutput> dxgiOutput;
            result = swapChain_->GetContainingOutput(&dxgiOutput); // 現在のスワップチェーンの出力デバイスを取得
            if (result == S_OK)
            {
                DXGI_OUTPUT_DESC outputDesc;
                result = dxgiOutput->GetDesc(&outputDesc); // 出力デバイスの解像度を取得する
                if (result == S_OK)
                {
                    fullscreenWindowRect = outputDesc.DesktopCoordinates; // フルスクリーンウィンドウの座標を設定する
                }
            }
        }

        if (result != S_OK)
        {
            // スワップチェーンからの解像度取得に失敗した場合、ディスプレイ設定を取得します。
            DEVMODE devmode = {};
            devmode.dmSize = sizeof(DEVMODE);
            EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);

            // 取得した表示解像度を使用してフルスクリーンウィンドウのサイズを決定してください
            fullscreenWindowRect = {
                devmode.dmPosition.x,
                devmode.dmPosition.y,
                devmode.dmPosition.x + static_cast<LONG>(devmode.dmPelsWidth),
                devmode.dmPosition.y + static_cast<LONG>(devmode.dmPelsHeight)
            };
        }
        // フルスクリーン用にウィンドウの位置とサイズを設定する
        SetWindowPos(
            hWnd,
            NULL,
            fullscreenWindowRect.left,
            fullscreenWindowRect.top,
            fullscreenWindowRect.right,
            fullscreenWindowRect.bottom,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ShowWindow(hWnd, SW_MAXIMIZE);
    }
    // 通常ウィンドウモード
    else
    {
        // ウィンドウモード設定
        DEVMODE devmode = {};
        devmode.dmSize = sizeof(DEVMODE);
        EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);

        // ウィンドウモードスタイルを設定する（最大化ボタンとサイズ変更を無効にする）
        DWORD windowedStyle = WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME | WS_VISIBLE;
        SetWindowLongPtrA(hWnd, GWL_STYLE, windowedStyle);
        SetWindowPos(
            hWnd,
            HWND_NOTOPMOST,
            windowedRect_.left,
            windowedRect_.top,
            windowedRect_.right - windowedRect_.left,
            windowedRect_.bottom - windowedRect_.top,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ShowWindow(hWnd, SW_NORMAL); // 通常のウィンドウとして表示する
    }
}

//  スワップチェーン作成
void Graphics::CreateSwapChain(IDXGIFactory6* dxgiFactory6, HWND hWnd)
{
    HRESULT result = S_OK;

    if (swapChain_)
    {
        // レンダーターゲットビュー開放
        renderTargetView_.Reset();
        //depthStencilView_.Reset();

        DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
        swapChain_->GetDesc(&swap_chain_desc);
        result = swapChain_->ResizeBuffers(swap_chain_desc.BufferCount, framebufferDimensions_.cx, framebufferDimensions_.cy, swap_chain_desc.BufferDesc.Format, swap_chain_desc.Flags);
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

        Microsoft::WRL::ComPtr<ID3D11Texture2D> render_target_buffer;
        result = swapChain_->GetBuffer(0, IID_PPV_ARGS(render_target_buffer.GetAddressOf()));
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
        D3D11_TEXTURE2D_DESC texture2d_desc;
        render_target_buffer->GetDesc(&texture2d_desc);

        result = device_->CreateRenderTargetView(render_target_buffer.Get(), NULL, renderTargetView_.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    }
    else
    {
        BOOL allowTearing = FALSE;
        result = dxgiFactory6_->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        tearingSupported_ = SUCCEEDED(result) && allowTearing;

        DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1{};
        swap_chain_desc1.Width              = framebufferDimensions_.cx;
        swap_chain_desc1.Height             = framebufferDimensions_.cy;
        swap_chain_desc1.Format             = DXGI_FORMAT_B8G8R8A8_UNORM;
        swap_chain_desc1.Stereo             = FALSE;
        swap_chain_desc1.SampleDesc.Count   = 1;
        swap_chain_desc1.SampleDesc.Quality = 0;
        swap_chain_desc1.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc1.BufferCount        = 2;
        swap_chain_desc1.Scaling            = DXGI_SCALING_STRETCH;
        swap_chain_desc1.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swap_chain_desc1.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
        swap_chain_desc1.Flags              = tearingSupported_ ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        result = dxgiFactory6->CreateSwapChainForHwnd(device_.Get(), hWnd, &swap_chain_desc1, NULL, NULL, swapChain_.ReleaseAndGetAddressOf());
#if 0
        swap_chain_desc1.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
#endif
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

        result = dxgiFactory6->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

        Microsoft::WRL::ComPtr<ID3D11Texture2D> render_target_buffer;
        result = swapChain_->GetBuffer(0, IID_PPV_ARGS(render_target_buffer.GetAddressOf()));
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
        result = device_->CreateRenderTargetView(render_target_buffer.Get(), NULL, renderTargetView_.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    }
    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(framebufferDimensions_.cx);
    viewport.Height = static_cast<float>(framebufferDimensions_.cy);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext_->RSSetViewports(1, &viewport);
}

// ビューポート作成
void Graphics::CreateViewport()
{
    // ビューポート再作成
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(framebufferDimensions_.cx);
    viewport.Height = static_cast<float>(framebufferDimensions_.cy);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext_->RSSetViewports(1, &viewport);
}

// レンダーターゲットビュー作成
void Graphics::CreateRenderTargetView()
{
    HRESULT result = S_OK;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer = {};

    result = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(backBuffer.GetAddressOf()));
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    result = device_->CreateRenderTargetView(backBuffer.Get(), NULL, renderTargetView_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

// デプスステンシルビュー作成
void Graphics::CreateDepthStencilView()
{
    HRESULT result = S_OK;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer = {};
    D3D11_TEXTURE2D_DESC texture2dDesc = {};
    texture2dDesc.Width                 = SCREEN_WIDTH;
    texture2dDesc.Height                = SCREEN_HEIGHT;
    texture2dDesc.MipLevels             = 1;
    texture2dDesc.ArraySize             = 1;
    texture2dDesc.Format                = DXGI_FORMAT_R24G8_TYPELESS;
    texture2dDesc.SampleDesc.Count      = 1;
    texture2dDesc.SampleDesc.Quality    = 0;
    texture2dDesc.Usage                 = D3D11_USAGE_DEFAULT;
    texture2dDesc.BindFlags             = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texture2dDesc.CPUAccessFlags        = 0;
    texture2dDesc.MiscFlags             = 0;

    result = device_->CreateTexture2D(&texture2dDesc, NULL, depthStencilBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
    depthStencilViewDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    depthStencilViewDesc.Flags              = 0;

    result = device_->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

//// 描画設定再設定(（ィンドウサイズ変更などによる描画設定の再設定）
//void Graphics::RecreateRenderTargets()
//{
//    HRESULT result = S_OK;
//
//    // レンダーターゲットビュー開放
//    renderTargetView_.Reset();
//    depthStencilView_.Reset();
//
//    // スワップチェーン設定取得
//    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
//    result = swapChain_->GetDesc(&swapChainDesc);
//    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));;
//
//    // スワップチェーン再設定
//    result = swapChain_->ResizeBuffers(swapChainDesc.BufferCount, framebufferDimensions_.cx, framebufferDimensions_.cy, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);
//    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
//
//    // レンダーターゲットビュー作成
//    Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTargetBuffer;
//    result = swapChain_->GetBuffer(0, IID_PPV_ARGS(renderTargetBuffer.GetAddressOf()));
//    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
//
//    D3D11_TEXTURE2D_DESC texture2d_desc;
//    renderTargetBuffer->GetDesc(&texture2d_desc);
//
//    result = device_->CreateRenderTargetView(renderTargetBuffer.Get(), NULL, renderTargetView_.ReleaseAndGetAddressOf());
//    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
//
//    // DepthStencilバッファ作成
//    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer = {};
//    D3D11_TEXTURE2D_DESC texture2dDesc = {};
//    texture2dDesc.Width                 = framebufferDimensions_.cx;
//    texture2dDesc.Height                = framebufferDimensions_.cy;
//    texture2dDesc.MipLevels             = 1;
//    texture2dDesc.ArraySize             = 1;
//    texture2dDesc.Format                = DXGI_FORMAT_R24G8_TYPELESS;
//    texture2dDesc.SampleDesc.Count      = 1;
//    texture2dDesc.SampleDesc.Quality    = 0;
//    texture2dDesc.Usage                 = D3D11_USAGE_DEFAULT;
//    texture2dDesc.BindFlags             = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
//    texture2dDesc.CPUAccessFlags        = 0;
//    texture2dDesc.MiscFlags             = 0;
//
//    result = device_->CreateTexture2D(&texture2dDesc, NULL, depthStencilBuffer.GetAddressOf());
//    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
//
//    // DepthStencilView作成
//    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
//    depthStencilViewDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
//    depthStencilViewDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
//    depthStencilViewDesc.Texture2D.MipSlice = 0;
//    depthStencilViewDesc.Flags              = 0;
//
//    result = device_->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView_.GetAddressOf());
//    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
//
//    // ビューポート再作成
//    D3D11_VIEWPORT viewport = {};
//    viewport.TopLeftX   = 0;
//    viewport.TopLeftY   = 0;
//    viewport.Width      = static_cast<float>(framebufferDimensions_.cx);
//    viewport.Height     = static_cast<float>(framebufferDimensions_.cy);
//    viewport.MinDepth   = 0.0f;
//    viewport.MaxDepth   = 1.0f;
//    deviceContext_->RSSetViewports(1, &viewport);
//}
