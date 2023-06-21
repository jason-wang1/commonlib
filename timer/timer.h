#pragma once

typedef void (*FONTIMER)(void *obj, int timer_id);

class CTimer
{
public:
    CTimer() {}
    ~CTimer() { Stop(); }
    void SetInterval(FONTIMER function, void *obj, int timerd, int interval);
    void Stop();

private:
    bool clear = false;
};
