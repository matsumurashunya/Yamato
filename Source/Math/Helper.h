#pragma once
#include <DirectXMath.h>

// ==================================================================================================
//                                               算術関数 
// ==================================================================================================

// XMFLOAT2正規化
inline auto VectorNomalize(const DirectX::XMFLOAT2& vector)
{
    DirectX::XMFLOAT2 rv = vector;
    DirectX::XMVECTOR Vector = DirectX::XMLoadFloat2(&rv);
    DirectX::XMStoreFloat2(&rv, DirectX::XMVector2Normalize(Vector));

    return rv;
}

// XMFLOAT3正規化
inline auto VectorNomalize(const DirectX::XMFLOAT3& vector)
{
    DirectX::XMFLOAT3 rv = vector;
    DirectX::XMVECTOR Vector = DirectX::XMLoadFloat3(&rv);
    DirectX::XMStoreFloat3(&rv, DirectX::XMVector3Normalize(Vector));

    return rv;
}

// 値を、決めた上限・下限の間で返す
inline float Clamp(float value, float min, float max)
{
    value = DirectX::XMMin(value, max);
    value = DirectX::XMMax(value, min);
}

// ０～１の間で値を返す
inline float Clamp01(float value, float min, float max)
{
    float normalizedValue;
    normalizedValue = (value - min) / (max - min);

    return normalizedValue;
}

// 開始地点から終了地点までのベクトルを返す(XMFLOAT2型)
inline DirectX::XMVECTOR& VectorFromTo(const DirectX::XMFLOAT2& startPoint, const DirectX::XMFLOAT2& endPoint)
{
    DirectX::XMVECTOR Start, End;
    Start = DirectX::XMLoadFloat2(&startPoint);
    End = DirectX::XMLoadFloat2(&endPoint);
    DirectX::XMVECTOR Vector = DirectX::XMVectorSubtract(End, Start);

    return Vector;
}

// 開始地点から終了地点までのベクトルを返す(XMFLOAT3型)
inline DirectX::XMVECTOR& VectorFromTo(const DirectX::XMFLOAT3& startPoint, const DirectX::XMFLOAT3& endPoint)
{
    DirectX::XMVECTOR Start, End;
    Start = DirectX::XMLoadFloat3(&startPoint);
    End = DirectX::XMLoadFloat3(&endPoint);
    DirectX::XMVECTOR Vector = DirectX::XMVectorSubtract(End, Start);

    return Vector;
}