#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <cstdint>

class FullscreenQuad
{
public:
    FullscreenQuad(ID3D11Device* device);
    ~FullscreenQuad() = default;

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> embeddedVertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> embeddedPixelShader_;

public:
    void Blit(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView** shaderResourceView, uint32_t startSlot, uint32_t numViews, ID3D11PixelShader* replacedPixelShader = nullptr);
};

