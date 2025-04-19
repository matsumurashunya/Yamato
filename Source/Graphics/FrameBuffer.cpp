#include "FrameBuffer.h"
#include "Misc.h"

FrameBuffer::FrameBuffer(ID3D11Device* device, uint32_t width, uint32_t height)
{
    HRESULT result = S_OK;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTargetBuffer;
    D3D11_TEXTURE2D_DESC texture2dDesc = {};
    texture2dDesc.Width = width;
    texture2dDesc.Height = height;
    texture2dDesc.MipLevels = 1;
    texture2dDesc.ArraySize = 1;
    texture2dDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texture2dDesc.SampleDesc.Count = 1;
    texture2dDesc.SampleDesc.Quality = 0;
    texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
    texture2dDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texture2dDesc.CPUAccessFlags = 0;
    texture2dDesc.MiscFlags = 0;

    result = device->CreateTexture2D(&texture2dDesc, 0, renderTargetBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));


    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
    renderTargetViewDesc.Format = texture2dDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    result = device->CreateRenderTargetView(renderTargetBuffer.Get(), &renderTargetViewDesc, renderTargetView_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));


    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    shaderResourceViewDesc.Format = texture2dDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    result = device->CreateShaderResourceView(renderTargetBuffer.Get(), &shaderResourceViewDesc, shaderResourceViews_[0].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));


    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBufffer = {};
    texture2dDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    result = device->CreateTexture2D(&texture2dDesc, 0, depthStencilBufffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));


    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Flags = 0;

    result = device->CreateDepthStencilView(depthStencilBufffer.Get(), &depthStencilViewDesc, depthStencilView_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

    result = device->CreateShaderResourceView(depthStencilBufffer.Get(), &shaderResourceViewDesc, shaderResourceViews_[1].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    viewport_.Width = static_cast<float>(width);
    viewport_.Height = static_cast<float>(height);
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;
    viewport_.TopLeftX = 0.0f;
    viewport_.TopLeftY = 0.0f;
}

// フレームバッファ初期化
void FrameBuffer::Clear(ID3D11DeviceContext* deviceContext, float r, float g, float b, float a, float depth)
{
    float color[4] = { r, g, b, a };
    deviceContext->ClearRenderTargetView(renderTargetView_.Get(), color);
    deviceContext->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH, depth, 0);
}

// フレームバッファ開始
void FrameBuffer::Activate(ID3D11DeviceContext* deviceContext)
{
    viewportCount_ = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    deviceContext->RSGetViewports(&viewportCount_, cachedViewports_);
    deviceContext->OMGetRenderTargets(1, cachedRenderTargetView_.ReleaseAndGetAddressOf(), cachedDepthStencilView_.ReleaseAndGetAddressOf());

    deviceContext->RSSetViewports(1, &viewport_);
    deviceContext->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());
}

// フレームバッファ終了
void FrameBuffer::Deactivate(ID3D11DeviceContext* deviceContext)
{
    deviceContext->RSSetViewports(viewportCount_, cachedViewports_);
    deviceContext->OMSetRenderTargets(1, cachedRenderTargetView_.GetAddressOf(), cachedDepthStencilView_.Get());

    cachedRenderTargetView_.Reset();
    cachedDepthStencilView_.Reset();

}
