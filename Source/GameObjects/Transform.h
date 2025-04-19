#pragma once
#include <DirectXMath.h>

class Object2D;
class Object3D;

// 2D用
class Transform2D
{
    friend Object2D;

public:
    Transform2D() = default;
    // ゲッター
    DirectX::XMFLOAT2 GetPosition() const { return position_; }
    DirectX::XMFLOAT2 GetSize()     const { return size_; }
    DirectX::XMFLOAT4 GetColor()    const { return color_; }
    DirectX::XMFLOAT2 GetTexSize()  const { return texSize_; }
    DirectX::XMFLOAT2 GetTexPos()   const { return texPos_; }
    DirectX::XMFLOAT2 GetPivot()    const { return pivot_; }
    float GetAngle()                const { return angle_; }

    // セッター
    void SetPosition(DirectX::XMFLOAT2 position) { position_ = position; }
    void SetSize(DirectX::XMFLOAT2 size) { size_ = size; }
    void SetColor(DirectX::XMFLOAT4 color) { color_ = color; }

private:
    DirectX::XMFLOAT2 position_     = { 0,0 };
    DirectX::XMFLOAT2 size_         = { 1,1 };
    DirectX::XMFLOAT4 color_        = { 1,1,1,1 };
    DirectX::XMFLOAT2 texSize_      = { 0,0 };
    DirectX::XMFLOAT2 texPos_       = { 0,0 };
    DirectX::XMFLOAT2 pivot_        = {};
    float             angle_        = 0;
};

// 3D用
class Transform3D
{
    friend Object3D;

public:
    enum class Axis
    {
        X,
        Y,
        Z
    };

public:
    // ゲッター
    DirectX::XMFLOAT3 GetRotation()         const { return rotation_; }
    DirectX::XMFLOAT3 GetScale()            const { return scale_; }
    DirectX::XMFLOAT3 GetPosition()         const { return position_; }
    DirectX::XMFLOAT4 GetQuaternion()       const { return orientation_; }
    DirectX::XMFLOAT4X4 GetWorldMatrix()    const { return worldMatrix_; }

    DirectX::XMFLOAT3 GetFrontVec();

    // セッター
    void SetRotation(DirectX::XMFLOAT3 rotation)     { rotation_ = rotation; }
    void SetRotationX(float rotationX)               { rotation_.x = rotationX; }
    void SetRotationY(float rotationY)               { rotation_.x = rotationY; }
    void SetRotationZ(float rotationZ)               { rotation_.x = rotationZ; }
    void SetScale(DirectX::XMFLOAT3 scale)           { scale_ = scale; }
    void SetScaleX(float scaleX)                     { scale_.x = scaleX; }
    void SetScaleY(float scaleY)                     { scale_.x = scaleY; }
    void SetScaleZ(float scaleZ)                     { scale_.x = scaleZ; }
    void SetPosition(DirectX::XMFLOAT3 position)     { position_ = position; }
    void SetPositionX(float positionX)               { position_.x = positionX; }
    void SetPositionY(float positionY)               { position_.x = positionY; }
    void SetPositionZ(float positionZ)               { position_.x = positionZ; }
    void SetTransform(DirectX::XMFLOAT4X4 matrix)    { worldMatrix_ = matrix; }

    // クォータニオン角度変更
    void RotationQuaternion(float angularSpeed/*角速度*/ , float elapsedTime, Axis axis)
    {
        DirectX::XMVECTOR Q = DirectX::XMLoadFloat4(&orientation_);
        DirectX::XMMATRIX M = DirectX::XMMatrixRotationQuaternion(Q);

        DirectX::XMVECTOR X, Y, Z;
        X = M.r[0];
        Y = M.r[1];
        Z = M.r[2];

        DirectX::XMVECTOR movement = DirectX::XMQuaternionIdentity();
        if (axis == Axis::X)
        {
            movement = DirectX::XMQuaternionRotationAxis(X, angularSpeed * elapsedTime);
        }
        else if (axis == Axis::Y)
        {
            movement = DirectX::XMQuaternionRotationAxis(Y, angularSpeed * elapsedTime);
        }
        else if (axis == Axis::Z)
        {
            movement = DirectX::XMQuaternionRotationAxis(Z, angularSpeed * elapsedTime);
        }
        Q = DirectX::XMQuaternionMultiply(Q, movement);

        Q = DirectX::XMQuaternionNormalize(Q);

        DirectX::XMStoreFloat4(&orientation_, Q);
    }

    // 回転モード切り替え
    void UseQuaternionRotation(bool enable) { useQuaternion_ = enable; }

    // 行列更新処理
    void UpdateTransform()
    {
        DirectX::XMMATRIX S, R, T, W;

        S = DirectX::XMMatrixScaling(scale_.x, scale_.y, scale_.z);                             // スケール行列
        if (useQuaternion_)
            R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&orientation_));      // 回転行列
        else
            R = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);   // 回転行列
        T = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);                // 位置行列
        W = S * R * T;
        DirectX::XMStoreFloat4x4(&worldMatrix_, W);
    }

private:
    DirectX::XMFLOAT3 position_     = { 0,0,0 };
    DirectX::XMFLOAT3 scale_        = { 1,1,1 };
    DirectX::XMFLOAT4 orientation_   = { 0,0,0,1 };
    DirectX::XMFLOAT3 rotation_     = { 0,0,0 };
    DirectX::XMFLOAT4X4 worldMatrix_ =
    {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };

    Axis axis_;

    bool useQuaternion_ = false;
};