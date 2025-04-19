#include "FullscreenQuad.h"
#include "Shader/Shader.h"
#include "Misc.h"
#include "Graphics.h"
#include "Shader/Shader.h"

FullscreenQuad::FullscreenQuad(ID3D11Device* device)
{
    Graphics::GetInstance().CreateVsFromCso("./Resources/Shader/FullscreenQuadVS.cso", embeddedVertexShader_.ReleaseAndGetAddressOf(), nullptr, nullptr, 0);
    Graphics::GetInstance().CreatePsFromCso("./Resources/Shader/FullscreenQuadPS.cso", embeddedPixelShader_.ReleaseAndGetAddressOf());
}

void FullscreenQuad::Blit(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView** shaderResourceView, uint32_t startSlot, uint32_t numViews, ID3D11PixelShader* replacedPixelShader)
{
    deviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    deviceContext->IASetInputLayout(nullptr);

    deviceContext->VSSetShader(embeddedVertexShader_.Get(), 0, 0);
    replacedPixelShader ? deviceContext->PSSetShader(replacedPixelShader, 0, 0) : deviceContext->PSSetShader(embeddedPixelShader_.Get(), 0, 0);

    deviceContext->PSSetShaderResources(startSlot, numViews, shaderResourceView);

    deviceContext->Draw(4, 0);
}
