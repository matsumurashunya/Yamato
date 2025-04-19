#pragma once
#include "./GameObjects/Transform.h"

class Object2D
{
public:
    Object2D() {};
    virtual ~Object2D() {}

    // 更新処理
    virtual void Update(float elapsedTime) = 0;

    // 描画処理
    virtual void Render() = 0;

    // デバッグ
    virtual void DrawDebug() = 0;

    // トランスフォームのゲッター
    const Transform2D& GetTransform() const { return transform_; }

private:
    Transform2D transform_ = {};
};