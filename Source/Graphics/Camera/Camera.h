#pragma once
#include <directXMath.h>

// カメラ
class Camera
{
public:
    Camera(); // コンストラクタ

    // ------------------- セッター --------------------

    void SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up);      // 指定方向に向く
    void SetPerspectiveFov(float fovY, float aspect, float nearZ, float farZ);                                      // パースペクティブ設定

    // ------------------- ゲッター --------------------

    const DirectX::XMFLOAT4X4& GetView()            const { return view_; }                // ビュー行列を取得
    const DirectX::XMFLOAT4X4& GetProjection()      const { return projection_; }          // プロジェクション行列を取得
    const DirectX::XMFLOAT3& GetCameraPosition()    const { return cameraPosition_; }      // 視点取得
    const DirectX::XMFLOAT3& GetForcus()            const { return focus_; }               // 注視点取得
    const DirectX::XMFLOAT3& GetUp()                const { return up_; }                  // 上方向取得
    const DirectX::XMFLOAT3& GetFront()             const { return front_; }               // 前方向取得
    const DirectX::XMFLOAT3& GetRight()             const { return right_; }               // 右方向取得

    void SetFocus(DirectX::XMFLOAT3& focus) { focus_ = focus; }
    void SetUp(DirectX::XMFLOAT3& up) { up_ = up; }
    void SetFront(DirectX::XMFLOAT3& front) { front_ = front; }
    void SetRight(DirectX::XMFLOAT3& right) { right_ = right; }
    void SetCameraPosition(DirectX::XMFLOAT3& cameraPosition) { cameraPosition_ = cameraPosition; }

private:
    DirectX::XMFLOAT4X4     view_ = {};
    DirectX::XMFLOAT4X4     projection_ = {};

    DirectX::XMFLOAT3       cameraPosition_ = {};
    DirectX::XMFLOAT3       focus_ = {};

    DirectX::XMFLOAT3       up_ = {};
    DirectX::XMFLOAT3       front_ = {};
    DirectX::XMFLOAT3       right_ = {};
};

