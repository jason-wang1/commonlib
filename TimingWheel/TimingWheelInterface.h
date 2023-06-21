#pragma once
#include <memory>
#include <functional>

namespace TimingWheel
{
    using TimeoutFunc = std::function<void()>;

    // we model 3 simple API for the construction and management of timers.
    //
    //  1. int Start(interval, expiry_action)
    //     start a timer that will expire after `interval` unit of time
    //
    //  2. void Cancel(timer_id)
    //    use `tiemr_id` to locate a timer and cancel it
    //
    //  3. int Tick(now)
    //    per-tick bookking routine
    //

    class TimingWheelInerface
    {
    public:
        // schedule a timer to run after specified time units(milliseconds).
        // returns an unique id identify this timer.
        virtual int Start(long ms, TimeoutFunc f) = 0;

        // cancel a timer by id
        // return true if successfully canceld
        virtual bool Cancel(int timer_id) = 0;

        // per-tick bookkeeping
        // return number of fired timers
        virtual int Tick(long now) = 0;

        // count of pending timers.
        virtual int Size() const = 0;

    protected:
        int nextId() { return next_id_++; }
        int next_id_ = 2020;
    };

}