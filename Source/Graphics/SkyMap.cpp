#include "SkyMap.h"
#include "Graphics.h"
#include "Texture.h"

SkyMap::SkyMap(const wchar_t* filename, bool generateMips)
{
    Graphics& graphics = Graphics::GetInstance();

    D3D11_TEXTURE2D_DESC texture2dDesc = {};
    LoadTextureFromFile(graphics.GetDevice(), filename, shaderResourceView_.GetAddressOf(), &texture2dDesc);

    if (texture2dDesc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
    {
        isTexturecube_ = true;
    }

    graphics.CreateVsFromCso("SkyMapVS.cso", skyMapVs_.GetAddressOf(), NULL, NULL, 0);
    graphics.CreatePsFromCso("SkyMapPS.cso", skyMapPs_.GetAddressOf());
    graphics.CreatePsFromCso("SkyBoxPS.cso", skyMapPs_.GetAddressOf());

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(Constants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    HRESULT result = graphics.GetDevice()->CreateBuffer(&bufferDesc, nullptr, constantBuffer_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

void SkyMap::Blit(const DirectX::XMFLOAT4X4& viewProjection)
{
    Graphics& graphics = Graphics::GetInstance();

    graphics.GetDeviceContext()->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
    graphics.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    graphics.GetDeviceContext()->IASetInputLayout(NULL);

    graphics.GetDeviceContext()->VSSetShader(skyMapVs_.Get(), 0, 0);
    graphics.GetDeviceContext()->PSSetShader(isTexturecube_ ? skyBoxPs_.Get() : skyMapPs_.Get(), 0, 0);

    graphics.GetDeviceContext()->PSSetShaderResources(0, 1, shaderResourceView_.GetAddressOf());

    Constants data;
    DirectX::XMStoreFloat4x4(&data.inverseViewProjection_, DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&viewProjection)));

    graphics.GetDeviceContext()->UpdateSubresource(constantBuffer_.Get(), 0, 0, &data, 0, 0);
    graphics.GetDeviceContext()->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());

    graphics.GetDeviceContext()->Draw(4, 0);

    graphics.GetDeviceContext()->VSSetShader(NULL, 0, 0);
    graphics.GetDeviceContext()->PSSetShader(NULL, 0, 0);
}
