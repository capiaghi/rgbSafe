#pragma once

#include <stdint.h>         /// uint
#include <arduino-timer.h>  /// Timer for RGB Strip or lock

class Lock
{
public:

    Lock();
    uint8_t initialize();
    uint8_t openLock(uint32_t);

private:
    Timer<1, millis> lockTimer;                       /// Timer: concurrent tasks, using millis as resolution

    uint8_t m_initStatus;                             /// Init done or not?
    static const uint32_t MAX_OPEN_TIME_MS = 10000;   /// Maximum open time of the lock in milliseconds
    static const uint32_t MIN_OPEN_TIME_MS = 1000;    /// Minimum open time of the lock in milliseconds
        
};
