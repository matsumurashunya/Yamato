#pragma once
#include <sstream>
#include "Graphics.h"
#include "Camera/Camera.h"
#include "HighResolutionTimer.h"
#include "DirectXMath.h"

class Framework
{
public:
    Framework(HWND hWnd);
    ~Framework();

    Framework(const Framework&) = delete;
    Framework& operator=(const Framework&) = delete;
    Framework(Framework&&) noexcept = delete;
    Framework& operator=(Framework&&) noexcept = delete;

    const int Run();
    LRESULT CALLBACK HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
    struct SceneConstants
    {
        DirectX::XMFLOAT4X4 viewProjection_;
        DirectX::XMFLOAT4   lightDirection_ = { 0,0,-1,0 };
        DirectX::XMFLOAT4   cameraPosition_;
    }sceneConstants_;

private:
    const bool Initialize();
    const bool Finalize();
    void Update(float elapsedTime);
    void Render(float elapsedTime);
    void CalculateFrameStats();

private:
    CONST HWND hWnd_;
    HighResolutionTimer tictoc_;
    uint32_t frames_ = 0;
    float totalTime_ = 0.0f;

    Camera camera_;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            constantBuffer_[8];
};

