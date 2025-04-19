#pragma onclass 

class CameraController
{
private:
    CameraController() {}
    ~CameraController() {}

public:

    static CameraController& GetInstance()
    {
        static CameraController instance;
        return instance;
    }

    void Initialize();                  // 初期化
    void Finalize();                    // 終了処理
    void Update(float elapsedTime, Camera& camera);     // 更新処理
    void DebugGUI();                    // デバッグ

    void CameraShake(float timer, float totalTime, float powerMin, float powerMax);

    // -------------------- セッター --------------------
    // -------------------- ゲッター --------------------

private:
    void FreeCamera(float elapsedTime, Camera& camera);             // フリーカメラ
    void TragetCamera(float elapsedTime);                           // ターゲットカメラ
    void ThirdPersonCamera(DirectX::XMFLOAT4X4& parentMatrix);      // 三人称カメラ
    void CircleCamera(float elapsedTime);                           // 円形カメラ

    void UECamera(Camera& camera);
    void UnityCamera(Camera& camera);
    void OriginCamera(Camera& camera);

private:
    enum class CameraType
    {
        Free,
        Target,
        TPC,
        Circle,
    };

    enum class FreeCameraType
    {
        UE,
        Unity,
        Origin,
    };

private:
    float distance_ = 10.0f;
    float angleX_;
    float angleY_;
    DirectX::XMFLOAT3 focus_;

    CameraType cameraType_ = CameraType::Free;
    FreeCameraType freeCamType_ = FreeCameraType::Origin;

};

