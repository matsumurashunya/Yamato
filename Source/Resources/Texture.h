#pragma once
//#include <wrl.h>
//#include <string>
//#include <map>
//#include <d3d11.h>

#include <wrl.h>
#include <string>
#include <map>
#include "Misc.h"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
using namespace Microsoft::WRL;
using namespace std;

static map<wstring, ComPtr<ID3D11ShaderResourceView>> resources;

// テクスチャのロード    
HRESULT LoadTextureFromFile(ID3D11Device* device, const wchar_t* fileName, ID3D11ShaderResourceView** shaderResourceView, D3D11_TEXTURE2D_DESC* rexture2dDesc);

HRESULT LoadTextureFromMemory(ID3D11Device* device, const void* data, size_t size, ID3D11ShaderResourceView** shaderResourceView);

// ダミーテクスチャの作成
HRESULT MakeDummyTexture(ID3D11Device* device, ID3D11ShaderResourceView** shaderResourceView, DWORD value, UINT dimension);

void ReleaseAllTextures();