#include "Shader.h"
#include "Graphics.h"
#include "Misc.h"

// コンストラクタ
Shader::Shader()
{
    // 各種ステート作成
    CreateBlendStates();        // ブレンドステート
    CreateDepthStencilStates(); // デプスステンシルステート
    CreateSamplerStates();      // サンプラーステート
    CreateRasterizerStates();   // ラスタライザーステート
}

// ブレンドステート設定
void Shader::SetBlendState(const BlendState& blendState)
{
    Graphics::GetInstance().GetDeviceContext()->OMSetBlendState(blendStates_[static_cast<UINT>(blendState)].Get(), nullptr, 0xFFFFFFFF);
}

// デプスステンシルステート設定
void Shader::SetDepthStencilState(const DepthState& depthState)
{
    Graphics::GetInstance().GetDeviceContext()->OMSetDepthStencilState(depthStencilStates_[static_cast<UINT>(depthState)].Get(), 1);
}

// サンプラーステート設定
void Shader::SetSamplerState()
{
    ID3D11DeviceContext* deviceContext = Graphics::GetInstance().GetDeviceContext();

    deviceContext->PSSetSamplers(0, 1, samplerStates_[static_cast<size_t>(SamplerState::Point)].GetAddressOf());
    deviceContext->PSSetSamplers(1, 1, samplerStates_[static_cast<size_t>(SamplerState::Linaer)].GetAddressOf());
    deviceContext->PSSetSamplers(2, 1, samplerStates_[static_cast<size_t>(SamplerState::Anisotropic)].GetAddressOf());
    deviceContext->PSSetSamplers(3, 1, samplerStates_[static_cast<size_t>(SamplerState::LinearBorderBlack)].GetAddressOf());
    //deviceContext->PSSetSamplers(4, 1, samplerStates_[static_cast<size_t>(SamplerState::LinearBorderWhite)].GetAddressOf());
    //deviceContext->PSSetSamplers(5, 1, samplerStates_[static_cast<size_t>(SamplerState::Comparison)].GetAddressOf());
}

// ラスタライザーステート設定
void Shader::SetRasterizerState(const RasterState& rasterState)
{
    Graphics::GetInstance().GetDeviceContext()->RSSetState(rasterizerStates_[static_cast<UINT>(rasterState)].Get());
}

// ブレンドステート作成
void Shader::CreateBlendStates()
{
	HRESULT result = S_OK;
	ID3D11Device* device = Graphics::GetInstance().GetDevice();

	D3D11_BLEND_DESC blendDesc = {};

	// NONE
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	result = device->CreateBlendState(&blendDesc, blendStates_[0].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

	// Alpha
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	result = device->CreateBlendState(&blendDesc, blendStates_[1].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

	// Add
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	result = device->CreateBlendState(&blendDesc, blendStates_[2].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

	// Multiply
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	result = device->CreateBlendState(&blendDesc, blendStates_[3].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

// デプスステンシルステート作成
void Shader::CreateDepthStencilStates()
{
	HRESULT result = S_OK;
	ID3D11Device* device = Graphics::GetInstance().GetDevice();

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};

	// 深度テスト　あり , 深度ライト　あり
	depthStencilDesc.DepthEnable	= TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc		= D3D11_COMPARISON_LESS_EQUAL;
	result = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates_[static_cast<int>(DepthState::TestAndWrite)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

	// 深度テスト　あり , 深度ライト　なし
	depthStencilDesc.DepthEnable	= TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc		= D3D11_COMPARISON_LESS_EQUAL;
	result = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates_[static_cast<int>(DepthState::Test)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

	// 深度テスト　なし , 深度ライト　あり
	depthStencilDesc.DepthEnable	= FALSE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc		= D3D11_COMPARISON_LESS_EQUAL;
	result = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates_[static_cast<int>(DepthState::Write)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

	// 深度テスト　なし , 深度ライト　なし
	depthStencilDesc.DepthEnable	= FALSE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc		= D3D11_COMPARISON_LESS_EQUAL;
	result = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates_[static_cast<int>(DepthState::NoTestNoWirte)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

// サンプラーステート作成
void Shader::CreateSamplerStates()
{
	HRESULT result = S_OK;
	ID3D11Device* device = Graphics::GetInstance().GetDevice();

	// Power Point 5 サンプラーステート見る！
	D3D11_SAMPLER_DESC samplerDesc = {};

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	result = device->CreateSamplerState(&samplerDesc, samplerStates_[0].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	result = device->CreateSamplerState(&samplerDesc, samplerStates_[1].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	result = device->CreateSamplerState(&samplerDesc, samplerStates_[2].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

// ラスタライザーステート作成
void Shader::CreateRasterizerStates()
{
	HRESULT result = S_OK;
	ID3D11Device* device = Graphics::GetInstance().GetDevice();

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = TRUE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	result = device->CreateRasterizerState(&rasterizerDesc, rasterizerStates_[0].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.AntialiasedLineEnable = TRUE;

	result = device->CreateRasterizerState(&rasterizerDesc, rasterizerStates_[1].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.AntialiasedLineEnable = TRUE;

	result = device->CreateRasterizerState(&rasterizerDesc, rasterizerStates_[2].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

// 頂点シェーダー
HRESULT Shader::CreateVsFromCso(const char* csoName, ID3D11VertexShader** vertexShader, ID3D11InputLayout** inputLayout,
	D3D11_INPUT_ELEMENT_DESC* inputElementDesc, UINT numElements)
{
	HRESULT result = S_OK;
	ID3D11Device* device = Graphics::GetInstance().GetDevice();

	FILE* fp = nullptr;
	fopen_s(&fp, csoName, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found.");

	fseek(fp, 0, SEEK_END);
	long csoSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> csoData = std::make_unique<unsigned char[]>(csoSize);
	fread(csoData.get(), csoSize, 1, fp);
	fclose(fp);

	result = device->CreateVertexShader(csoData.get(), csoSize, nullptr, vertexShader);
	_ASSERT_EXPR_A(SUCCEEDED(result), HRTrace(result));

	if (inputLayout)
	{
		result = device->CreateInputLayout(inputElementDesc, numElements, csoData.get(), csoSize, inputLayout);
		_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
	}

	return result;
}

// ピクセルシェーダー
HRESULT Shader::CreatePsFromCso(const char* csoName, ID3D11PixelShader** pixelShader)
{
	HRESULT result = S_OK;
	ID3D11Device* device = Graphics::GetInstance().GetDevice();

	FILE* fp = nullptr;
	fopen_s(&fp, csoName, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found.");

	fseek(fp, 0, SEEK_END);
	long csoSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]>cso_data = std::make_unique<unsigned char[]>(csoSize);
	fread(cso_data.get(), csoSize, 1, fp);
	fclose(fp);

	result = device->CreatePixelShader(cso_data.get(), csoSize, nullptr, pixelShader);
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

	return result;
}
