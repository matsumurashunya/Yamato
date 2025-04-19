#pragma once
#include <memory>
#include "Systems/GamePad.h"
#include "Systems/Mouse.h"

class Input
{
private:
    Input() {}
    ~Input() {}

public:
    static Input& GetInstance()
    {
        static Input instance;
        return instance;
    }

    // ‰Šú‰»
    void Initialize(HWND hWnd);

    // XVˆ—
    void Update();

private:
    std::unique_ptr<GamePad>    gamePad_;
    std::unique_ptr<Mouse>      mouse_;
};

