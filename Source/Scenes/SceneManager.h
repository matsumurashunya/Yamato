#pragma once
#include "BaseScene.h"

class SceneManager
{
private:
    SceneManager() {}
    ~SceneManager() {}

public:
    static SceneManager& GetInstance()
    {
        static SceneManager instance;
        return instance;
    }

    void Initialize();                  // 初期化
    void Finalize();                    // 終了化
    void Update(float elapsedTime);     // 更新処理
    void Render();                      // 描画処理
    void DrawDebug();                   // デバッグ

    void Clear();                           // Scene初期化
    void ChangeScene(BaseScene* scene);     // Scene切り替え

private:
    BaseScene* currentScene_    = nullptr;
    BaseScene* nextScene_       = nullptr;
};

