#pragma once
#include "Camera/Camera.h"
#include <cstdlib>
#include <algorithm>
#include <windows.h>
#include "CameraController.h"
#include "ImGui/imgui.h"

// 初期化
void CameraController::Initialize()
{
}

// 終了処理
void CameraController::Finalize()
{
}

// 更新処理
void CameraController::Update(float elapsedTime, Camera& camera)
{
	switch (cameraType_)
	{
	case CameraType::Free:
		FreeCamera(elapsedTime, camera);

		break;
	case CameraType::TPC:
		//ThirdPersonCamera();

		break;
	case CameraType::Target:
		TragetCamera(elapsedTime);

		break;
	case CameraType::Circle:
		CircleCamera(elapsedTime);

		break;
	}
}

// フリーカメラ
void CameraController::FreeCamera(float elapsedTime, Camera& camera)
{
	// デバッグウインドウ操作中は処理しない
	//if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
	//{
	//	return;
	//}
	switch (freeCamType_)
	{
	case FreeCameraType::UE:
		UECamera(camera);
		break;

	case FreeCameraType::Unity:
		UnityCamera(camera);
		break;

	case FreeCameraType::Origin:
		OriginCamera(camera);
		break;
	}

}

// ターゲットカメラ
void CameraController::TragetCamera(float elapsedTime)
{
}

// 三人称カメラ
void CameraController::ThirdPersonCamera(DirectX::XMFLOAT4X4& parentMatrix)
{
}

// 円形カメラ
void CameraController::CircleCamera(float elapsedTime)
{
}

// -------------------- フリーカメラ用 --------------------
void CameraController::UECamera(Camera& camera)
{
	float moveX = 0, moveY = 0;
	static float pertical, horizontal;
	ImGuiIO io = ImGui::GetIO();

	if (GetAsyncKeyState(0x51))pertical = 10.0f;
	if (GetAsyncKeyState(0x45))pertical = -10.0f;

	// 垂直移動
	if (io.MouseDown[ImGuiMouseButton_Right])
	{
		// 垂直移動
		float s = distance_ * 0.035f;
		float y = (moveY + pertical) * s;

		focus_.x += camera.GetUp().x * y;
		focus_.y += camera.GetUp().y * y;
		focus_.z += camera.GetUp().z * y;
	}

}

void CameraController::UnityCamera(Camera& camera)
{

}
void CameraController::OriginCamera(Camera& camera)
{
	// IMGUIのマウス入力値を使ってカメラ操作する
	ImGuiIO io = ImGui::GetIO();

	// マウスカーソルの移動量を求める
	float moveX = io.MouseDelta.x * 0.02f;
	float moveY = io.MouseDelta.y * 0.02f;

	// マウス左ボタン押下中
	if (io.MouseDown[ImGuiMouseButton_Right])
	{
		// Y軸回転
		angleY_ += moveX * 0.5f;
		if (angleY_ > DirectX::XM_PI)
		{
			angleY_ -= DirectX::XM_2PI;
		}
		else if (angleY_ < -DirectX::XM_PI)
		{
			angleY_ += DirectX::XM_2PI;
		}
		// X軸回転
		angleX_ += moveY * 0.5f;
		if (angleX_ > DirectX::XM_PI)
		{
			angleX_ -= DirectX::XM_2PI;
		}
		else if (angleX_ < -DirectX::XM_PI)
		{
			angleX_ += DirectX::XM_2PI;
		}
	}
	// マウス中ボタン押下中
	else if (io.MouseDown[ImGuiMouseButton_Middle])
	{
		// 平行移動
		float s = distance_ * 0.035f;
		float x = moveX * s;
		float y = moveY * s;

		focus_.x -= camera.GetRight().x * x;
		focus_.y -= camera.GetRight().y * x;
		focus_.z -= camera.GetRight().z * x;

		focus_.x += camera.GetUp().x * y;
		focus_.y += camera.GetUp().y * y;
		focus_.z += camera.GetUp().z * y;
	}
	// マウス右ボタン押下中
	else if (io.MouseDown[ImGuiMouseButton_Left] && io.MouseDown[ImGuiMouseButton_Right])
	{
		// ズーム
		distance_ += (-moveY - moveX) * distance_ * 0.1f;
	}
	// マウスホイール
	else if (io.MouseWheel != 0)
	{
		// ズーム
		distance_ -= io.MouseWheel * distance_ * 0.1f;
	}

	float sx = ::sinf(angleX_);
	float cx = ::cosf(angleX_);
	float sy = ::sinf(angleY_);
	float cy = ::cosf(angleY_);

	// カメラの方向を算出
	DirectX::XMVECTOR Front = DirectX::XMVectorSet(-cx * sy, -sx, -cx * cy, 0.0f);
	DirectX::XMVECTOR Right = DirectX::XMVectorSet(cy, 0, -sy, 0.0f);
	DirectX::XMVECTOR Up = DirectX::XMVector3Cross(Right, Front);
	// カメラの視点＆注視点を算出
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus_);
	DirectX::XMVECTOR Distance = DirectX::XMVectorSet(distance_, distance_, distance_, 0.0f);
	DirectX::XMVECTOR Eye = DirectX::XMVectorSubtract(Focus, DirectX::XMVectorMultiply(Front, Distance));
	// ビュー行列からワールド行列を算出
	DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
	DirectX::XMMATRIX World = DirectX::XMMatrixTranspose(View);
	// ワールド行列から方向を算出
	Right = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(1, 0, 0, 0), World);
	Up = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 1, 0, 0), World);
	// 結果を格納
	DirectX::XMFLOAT3 eye, up, right;//focus;
	DirectX::XMStoreFloat3(&eye, Eye);
	DirectX::XMStoreFloat3(&up, Up);
	DirectX::XMStoreFloat3(&right, Right);

	camera.SetCameraPosition(eye);
	camera.SetUp(up);
	camera.SetRight(right);

	DirectX::XMStoreFloat3(&focus_, Focus);
	camera.SetLookAt(eye, focus_, up);
}
// -------------------- 終わり --------------------

// デバッグ
void CameraController::DebugGUI()
{
	using namespace ImGui;

	//if(TreeNode(u8"カメラ"))
	{
		//DragFloat3(u8"座標", &, 0.1f, 0);
		DragFloat(u8"距離", &distance_, 0.1f, 0.1f);
	}

	//ImGui::TreePop();
}

// カメラシェイク
void CameraController::CameraShake(float timer, float totalTime, float powerMin, float powerMax)
{
}