#pragma once
#include "BaseScene.h"

//TODO テスト
#include "Transform.h"
#include "Model.h"
#include <memory>
#include "SkyMap.h"

class TestScene : public BaseScene
{
public:
    TestScene() {}
    ~TestScene() {}

    // コンストラクタ
    void Initialize() override;

    // デストラクタ
    void Finalize() override;

    // 更新処理
    void Update(float elapsedTime);

    // 描画処理
    void Render();

    // デバッグ
    void DrawDebug();

private:
    std::unique_ptr<SkyMap> skyMap_;
    std::unique_ptr<Model> model_;
    Transform3D object;
};

