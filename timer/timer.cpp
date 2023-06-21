#include "timer.h"
#include <thread>
#include <chrono>

void CTimer::SetInterval(FONTIMER function, void *obj, int timer_id, int interval)
{
    this->clear = false;
    std::thread t([=]() {
        while (true)
        {
            if (this->clear)
            {
                return;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(interval));

            if (this->clear)
            {
                return;
            }
            function(obj, timer_id);
        }
    });

    t.detach();
}

void CTimer::Stop()
{
    this->clear = true;
}