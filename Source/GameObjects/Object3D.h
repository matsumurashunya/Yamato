#pragma once
#include "GameObjects/Transform.h"

class Object3D
{
public:
    Object3D() {}
    virtual ~Object3D() {}

    // 更新処理
    virtual void Update(float elapsedTime) = 0;

    // 描画処理
    virtual void Render() = 0;

    // デバッグ
    virtual void DrawDebug() = 0;

    // トランスフォームのゲッター
    const Transform3D& GetTransform() const { return transform_; }

private:
    Transform3D transform_ = {};
};
