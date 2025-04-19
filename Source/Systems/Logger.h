#pragma once
#include <Windows.h>
#include <string>

class Logger
{
public:
    enum class Level
    {
        Info,       // 情報
        Warning,    // 注意
        Error       // エラー
    };

    // デバッグログ（ワイド文字）
    static void Log(const std::wstring& message, Level level)
    {
        level_ = level;
        switch (level_)
        {
        case Level::Info:
            return OutputDebugStringW((L"[ Info ]  " + message + L"\n").c_str());
        case Level::Warning:
            return OutputDebugStringW((L"[ Warning ]  " + message + L"\n").c_str());
        case Level::Error:
            return OutputDebugStringW((L"[ Error ]  " + message + L"\n").c_str());
        default:
            return;
        }
    }

private:
    inline static Level level_ = Level::Info;
};

