#include "SceneManager.h"

// 各シーン
#include "TestScene.h"

// 初期化
void SceneManager::Initialize()
{
    ChangeScene(new TestScene);
}

// 終了化
void SceneManager::Finalize()
{
    Clear();
}

// 更新処理
void SceneManager::Update(float elapsedTime)
{
    if (nextScene_)
    {
        Clear();

        // 新しいシーンを設定
        currentScene_ = nextScene_;
        nextScene_ = nullptr;

        currentScene_->Initialize();
    }

    if (currentScene_)
    {
        currentScene_->Update(elapsedTime);
    }
}

// 描画処理
void SceneManager::Render()
{
    if (currentScene_ == nullptr)return;

    currentScene_->Render();
}

// デバッグ
void SceneManager::DrawDebug()
{
    if (currentScene_ == nullptr) return;

    currentScene_->DrawDebug();
}

// Sceneのクリア
void SceneManager::Clear()
{
    if (currentScene_ == nullptr) return;

    currentScene_->Finalize();
    delete currentScene_;
    currentScene_ = nullptr;
}

// Sceneの知り替え
void SceneManager::ChangeScene(BaseScene* scene)
{
    nextScene_ = scene;
}
