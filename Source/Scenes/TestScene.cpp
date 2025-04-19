#include "TestScene.h"
#include "Graphics.h"

#include "Systems/Logger.h"

// コンストラクタ
void TestScene::Initialize()
{
    //skyMap_ = std::make_unique<SkyMap>(L".\\Resources\\winter_evening_4k.DDS");
    //model_ = std::make_unique<Model>("./Resources/Model/DamagedHelmet/glTF/DamagedHelmet.gltf");
    model_ = std::make_unique<Model>("./Resources/Model/CesiumMan/glTF/CesiumMan.gltf");
    //model_ = std::make_unique<Model>("./Resources/Model/BrainStem/glTF/BrainStem.gltf");
}

// デストラクタ
void TestScene::Finalize()
{
}
static float elapsedTime_ = 0;
// 更新処理
void TestScene::Update(float elapsedTime)
{
    DirectX::XMFLOAT3 scale = { 1,1,1 };
    object.SetScale(scale);
    object.UpdateTransform();

    Logger::Log(L"今日は寝ます", Logger::Level::Warning);
    elapsedTime_ = elapsedTime;
}

// 描画処理
void TestScene::Render()
{
    Graphics::GetInstance().frameBuffer_[0]->Clear(Graphics::GetInstance().GetDeviceContext(),0,0,1,1);
    Graphics::GetInstance().frameBuffer_[0]->Activate(Graphics::GetInstance().GetDeviceContext());

    // 3D描画
    {
        Set3DRendering();

        static std::vector<Model::Node> animated_nodes = model_->nodes_;
#if 1
        static float time{ 0 };
        model_->Animate(0/*animation index*/, time += elapsedTime_, animated_nodes);
        if (model_->animations_.at(0/*animation index*/).duration_ < time)
        {
            time = 0; // Repeat playback 
        }
#endif
        model_.get()->Render(object.GetWorldMatrix(), animated_nodes);
        //model_.get()->Render(object.GetWorldMatrix(), );
    }

    // 2D描画
    {
        Set2DRendeing();
        //skyMap_->Blit();

    }

    Graphics::GetInstance().frameBuffer_[0]->Deactivate(Graphics::GetInstance().GetDeviceContext());
    Graphics::GetInstance().frameBuffer_[1]->Clear(Graphics::GetInstance().GetDeviceContext());
    Graphics::GetInstance().frameBuffer_[1]->Activate(Graphics::GetInstance().GetDeviceContext());
    Graphics::GetInstance().bitBlockTransfer_->Blit(Graphics::GetInstance().GetDeviceContext(), Graphics::GetInstance().frameBuffer_[0]->shaderResourceViews_[0].GetAddressOf(), 0, 1, nullptr/*Graphics::GetInstance().pixelShaders_[0].Get()*/);
    Graphics::GetInstance().frameBuffer_[1]->Deactivate(Graphics::GetInstance().GetDeviceContext());

    Graphics::GetInstance().Screen();
}

// デバッグ
void TestScene::DrawDebug()
{
}
