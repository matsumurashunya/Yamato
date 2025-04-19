#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

class SkyMap
{
public:
    SkyMap(const wchar_t* filename, bool generateMips = false);
    ~SkyMap() = default;

    void Blit(const DirectX::XMFLOAT4X4& viewProjection);

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView_;

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> skyMapVs_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> skyMapPs_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> skyBoxPs_;

    struct Constants
    {
        DirectX::XMFLOAT4X4 inverseViewProjection_;
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;

    bool isTexturecube_ = false;
};

