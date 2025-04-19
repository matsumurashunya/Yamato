#pragma once
#include <Windows.h>

class HighResolutionTimer
{
public:
    HighResolutionTimer()
    {
        LONGLONG countPerSec;
        QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countPerSec));
        secondsPerCount_ = 1.0 / static_cast<double>(countPerSec);

        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime_));
        baseTime_ = thisTime_;
        lastTime_ = thisTime_;
    }
    ~HighResolutionTimer() = default;
    HighResolutionTimer(const HighResolutionTimer&) = delete;
    HighResolutionTimer& operator=(const HighResolutionTimer&) = delete;
    HighResolutionTimer(HighResolutionTimer&&) noexcept = delete;
    HighResolutionTimer& operator=(HighResolutionTimer&&) noexcept = delete;

    // Reset() が呼び出されてからの経過時間を戻り値として返す
    float TimeStamp() const
    {
        // If we are stopped, do not count the time that has passed since we stopped.
        // Moreover, if we previously already had a pause, the distance 
        // stop_time - base_time includes paused time, which we do not want to count.
        // To correct this, we can subtract the paused time from mStopTime:  
        // 
        //                     |<--paused_time-->|
        // ----*---------------*-----------------*------------*------------*------> time
        //  base_time       stop_time        start_time     stop_time    this_time
        if (stopped_)
        {
            return static_cast<float>(((stopTime_ - pausedTime_) - baseTime_) * secondsPerCount_);
        }
        // The distance this_time - mBaseTime includes paused time,
        // which we do not want to count.  To correct this, we can subtract 
        // the paused time from this_time:  
        //
        //  (this_time - paused_time) - base_time 
        //
        //                     |<--paused_time-->|
        // ----*---------------*-----------------*------------*------> time
        //  base_time       stop_time        start_time     this_time
        else
        {
            return static_cast<float>(((thisTime_ - pausedTime_) - baseTime_) * secondsPerCount_);
        }
    }

    float TimeInterval() const
    {
        return static_cast<float>(deltaTime_);
    }

    // メッセージループの前に呼び出す
    void Reset()
    {
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime_));
        baseTime_ = thisTime_;
        lastTime_ = thisTime_;

        stopTime_ = 0;
        stopped_  = false;
    }

    // 一時停止が解除された時に呼び出す
    void Start() 
    {
        LONGLONG startTime;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&startTime));

        // Accumulate the time elapsed between stop and start pairs.
        //
        //                     |<-------d------->|
        // ----*---------------*-----------------*------------> time
        //  base_time       stop_time        start_time     
        if (stopped_)
        {
            pausedTime_ += (startTime - stopTime_);
            lastTime_ = startTime;
            stopTime_ = 0;
            stopped_ = false;
        }
    }

    // 一時停止するときに呼ぶ
    void Stop()
    {
        if (stopped_ == false)
        {
            QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&stopTime_));
            stopped_ = true;
        }
    }

    // 毎フレーム呼ぶ
    void Tick() 
    {
        if (stopped_)
        {
            deltaTime_ = 0.0f;
            return;
        }

        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime_));

        // このフレームと前のフレーム間の時間差
        deltaTime_ = (thisTime_ - lastTime_) * secondsPerCount_;

        // 次のフレームの準備
        lastTime_ = thisTime_;

        // Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
        // processor goes into a power save mode or we get shuffled to another
        // processor, then mDeltaTime can be negative.
        if (deltaTime_ < 0.0)
        {
            deltaTime_ = 0.0;
        }
    }

private:
    double secondsPerCount_ = 0.0;
    double deltaTime_       = 0.0;
    LONGLONG baseTime_      = 0LL;
    LONGLONG pausedTime_    = 0LL;
    LONGLONG stopTime_      = 0LL;
    LONGLONG lastTime_      = 0LL;
    LONGLONG thisTime_      = 0LL;
    bool stopped_           = false;
};