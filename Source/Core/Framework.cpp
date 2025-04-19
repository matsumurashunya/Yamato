#include "Framework.h"
#include "Scenes/SceneManager.h"
#include "Systems/CameraController.h"
#include "ImGui/ImGuiCtrl.h"
#include "Misc.h"

// コンストラクタ
Framework::Framework(HWND hWnd) : hWnd_(hWnd)
{
}

// デストラクタ
Framework::~Framework()
{
}

// 初期化
const bool Framework::Initialize()
{
	// グラフィックス初期化
	Graphics::GetInstance().Initialize(hWnd_);

	HRESULT result = S_OK;

	// コンスタントバッファはGPUに情報を送る、画面がシーン
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(SceneConstants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	result = Graphics::GetInstance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, constantBuffer_[0].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

	// カメラの初期設定
	camera_.SetPerspectiveFov(DirectX::XMConvertToRadians(30), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 1000.0f);
	camera_.SetLookAt(
		{ 0,0,-10 },	// 視点
		{ 0,0,0 },	// 注視点
		{ 0,1,0 }	// 上ベクトル
	);

	SceneManager::GetInstance().Initialize();

    return true;
}

// 終了化
const bool Framework::Finalize()
{
	SceneManager::GetInstance().Finalize();

    return true;
}

// 更新処理
void Framework::Update(float elapsedTime)
{
	SceneManager::GetInstance().Update(elapsedTime);

	// ImGui初期化
	IMGUI_CTRL_CLEAR_FRAME();

	if (GetAsyncKeyState(VK_RETURN) & 0x0001 && GetAsyncKeyState(VK_MENU) & 0x8000)
	{
		Graphics::GetInstance().StylizeWndiow(hWnd_, !Graphics::GetInstance().GetFullscreen());
	}

	Graphics::GetInstance().GetDeviceContext()->UpdateSubresource(constantBuffer_[0].Get(), 0, 0, &sceneConstants_, 0, 0);
	Graphics::GetInstance().GetDeviceContext()->VSSetConstantBuffers(1, 1, constantBuffer_[0].GetAddressOf());
	Graphics::GetInstance().GetDeviceContext()->PSSetConstantBuffers(1, 1, constantBuffer_[0].GetAddressOf());

#ifdef USE_IMGUI
	SceneManager::GetInstance().DrawDebug();
#endif
}

// 描画処理
void Framework::Render(float elapsedTime)
{
	// 描画初期化
	Graphics::GetInstance().Clear(0,0,1);

	// カメラ
	CameraController::GetInstance().Update(elapsedTime, camera_);

	DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&camera_.GetView());
	DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4(&camera_.GetProjection());
	DirectX::XMStoreFloat4x4(&sceneConstants_.viewProjection_, view * projection);

	SceneManager::GetInstance().Render();

	// ImGui描画
	IMGUI_CTRL_DISPLAY();

	// 描画
	Graphics::GetInstance().Present();
}

// 実行
const int Framework::Run()
{
	MSG msg = {};
	Graphics& graphics = Graphics::GetInstance();

	if (Initialize() == false) return 0;


	IMGUI_CTRL_INITIALIZE(hWnd_, graphics.GetDevice(), graphics.GetDeviceContext());

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			tictoc_.Tick();
			CalculateFrameStats();
			Update(tictoc_.TimeInterval());
			Render(tictoc_.TimeInterval());
		}
	}

	// ImGui終了化
	IMGUI_CTRL_UNINITIALIZE();

#if 0
	BOOL fullscreen = 0;
	graphics.GetSwapChain()->GetFullscreenState(&fullscreen, 0);
	if (fullscreen)
	{
		graphics.GetSwapChain()->SetFullscreenState(FALSE, 0);
	}
#endif

	return Finalize() ? static_cast<int>(msg.wParam) : 0;
}

// メッセージハンドラ
LRESULT Framework::HandleMessage(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// ImGuiハンドラ
	IMGUI_CTRL_WND_PRC_HANDLER(hWnd, msg, wparam, lparam);

	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps = {};
		BeginPaint(hWnd, &ps);

		EndPaint(hWnd, &ps);
	}
	break;

	case WM_DESTROY:		// ウィンドウ終了時
		PostQuitMessage(0);
		break;

	case WM_CREATE:			// ウィンドウ生成時
		break;

	case WM_KEYDOWN:		// キーが押されたとき
		//ESCキーで終了処理
		if (wparam == VK_ESCAPE)
		{
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
		break;

	case WM_ENTERSIZEMOVE:	// ウィンドウの移動時
		tictoc_.Stop();
		break;

	case WM_EXITSIZEMOVE:	// ウィンドウの移動終了時
		tictoc_.Start();
		break;

	case WM_SIZE:			// ウィンドウサイズ変更
	{
		RECT clientRect{};
		GetClientRect(hWnd, &clientRect);
		Graphics::GetInstance().OnResize(static_cast<UINT64>(clientRect.right - clientRect.left), clientRect.bottom - clientRect.top, hWnd);
		break;
	}
	default:				// Windowsデフォルトの処理呼び出し
		return DefWindowProc(hWnd, msg, wparam, lparam);
	}
	return 0;
}

// フレーム計算
void Framework::CalculateFrameStats()
{
    if (++frames_, (tictoc_.TimeStamp() - totalTime_) >= 1.0f)
    {
        float fps = static_cast<float>(frames_);
        std::wostringstream outs;
        outs.precision(6);

        // アプリケーション名
        outs << APPLICATION_NAME;

        // FPS表示
        outs << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";

        SetWindowTextW(hWnd_, outs.str().c_str());

        frames_ = 0;
        totalTime_ += 1.0f;
    }
}
