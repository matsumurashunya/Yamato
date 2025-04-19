#include "Texture.h"
#include <memory>
using namespace Microsoft::WRL;
using namespace std;

HRESULT LoadTextureFromFile(ID3D11Device* device, const wchar_t* fileName, ID3D11ShaderResourceView** shaderResourceView, D3D11_TEXTURE2D_DESC* rexture2dDesc)
{
    HRESULT result = S_OK;
    ComPtr<ID3D11Resource> resource;

    auto it = resources.find(fileName);
    if (it != resources.end())
    {
        *shaderResourceView = it->second.Get();
        (*shaderResourceView)->AddRef();
        (*shaderResourceView)->GetResource(resource.GetAddressOf());
    }
    else
    {
        result = DirectX::CreateWICTextureFromFile(device, fileName, resource.GetAddressOf(), shaderResourceView);
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
        resources.insert(make_pair(fileName, *shaderResourceView));
    }

    ComPtr<ID3D11Texture2D> texture2d;
    result = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    texture2d->GetDesc(rexture2dDesc);

    return result;
}

HRESULT LoadTextureFromMemory(ID3D11Device* device, const void* data, size_t size, ID3D11ShaderResourceView** shaderResourceView)
{
    HRESULT result = S_OK;
    ComPtr<ID3D11Resource> resource;

    result = DirectX::CreateDDSTextureFromMemory(device, reinterpret_cast<const uint8_t*>(data), size, resource.GetAddressOf(), shaderResourceView);
    if (result != S_OK)
    {
        result = DirectX::CreateWICTextureFromMemory(device, reinterpret_cast<const uint8_t*>(data), size, resource.GetAddressOf(), shaderResourceView);
    }

    return result;
}

HRESULT MakeDummyTexture(ID3D11Device* device, ID3D11ShaderResourceView** shaderResourceView, DWORD value, UINT dimension)
{
    HRESULT result = S_OK;

    D3D11_TEXTURE2D_DESC texture2dDesc = {};
    texture2dDesc.Width = dimension;
    texture2dDesc.Height = dimension;
    texture2dDesc.MipLevels = 1;
    texture2dDesc.ArraySize = 1;
    texture2dDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texture2dDesc.SampleDesc.Count = 1;
    texture2dDesc.SampleDesc.Quality = 0;
    texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
    texture2dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    size_t texels = dimension * dimension;
    unique_ptr<DWORD[]> sysmem{ make_unique<DWORD[]>(texels) };
    for (size_t i = 0; i < texels; ++i)sysmem[i] = value;

    D3D11_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pSysMem = sysmem.get();
    subresourceData.SysMemPitch = sizeof(DWORD) * dimension;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
    result = device->CreateTexture2D(&texture2dDesc, &subresourceData, &texture2d);
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    shaderResourceViewDesc.Format = texture2dDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    result = device->CreateShaderResourceView(texture2d.Get(), &shaderResourceViewDesc, shaderResourceView);
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    return result;
}

void ReleaseAllTextures()
{
    resources.clear();
}
