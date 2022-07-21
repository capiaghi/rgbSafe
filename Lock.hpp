#pragma once

#include <stdint.h>

class Lock
{
public:

    Lock();
    uint8_t initialize();
    uint8_t openLock(uint32_t);

private:
    static const uint32_t MAX_OPEN_TIME_MS = 10000;   // Maximum open time of the lock in milliseconds
    static const uint32_t MIN_OPEN_TIME_MS = 1000;    // Minimum open time of the lock in milliseconds
        
};
