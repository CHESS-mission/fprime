/**
 * Posix/IntervalTimer.cpp:
 *
 * The Posix implementation of the interval timer shares the same raw setup as
 * other X86 implementations. That is: the lower U32 of the RawTime is
 * nano-seconds, and the upper U32 of RawTime object is seconds. Thus only the
 * "getRawTime" function differes from the base X86 version of this file.
 */
#include <errno.h>
#include <time.h>

#include <Fw/Types/Assert.hpp>
#include <Os/IntervalTimer.hpp>

#include "FreeRTOS.h"

namespace Os {

IntervalTimer::IntervalTimer() {
    memset(&this->m_startTime, 0, sizeof(this->m_startTime));
    memset(&this->m_stopTime, 0, sizeof(this->m_stopTime));
}

IntervalTimer::~IntervalTimer() {}

U32 IntervalTimer::getTimerFrequency(void) { return 1; }

void IntervalTimer::getRawTime(RawTime& time) {
    TickType_t xTime1;

    /* Get the time the function started. */

    xTime1 = xTaskGetTickCount();
    time.lower = 1000 * pdMS_TO_TICKS(xTime1);
    time.upper = 0;
}

U32 IntervalTimer::getDiffUsec(void) {
    return getDiffUsec(this->m_stopTime, this->m_startTime);
}

// Adapted from:
// http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html

// should be t1In - t2In

U32 IntervalTimer::getDiffUsec(const RawTime& t1In, const RawTime& t2In) {
    RawTime result = {t1In.upper - t2In.upper, 0};

    if (t1In.lower < t2In.lower) {
        result.upper -= 1;  // subtract nsec carry to seconds
        result.lower = t1In.lower + (1000000000 - t2In.lower);
    } else {
        result.lower = t1In.lower - t2In.lower;
    }

    return result.lower;
}

void IntervalTimer::start() { getRawTime(this->m_startTime); }

void IntervalTimer::stop() { getRawTime(this->m_stopTime); }

IntervalTimer::RawTime IntervalTimer::getDiffRaw(const RawTime& t1,
                                                 const RawTime& t2) {
    RawTime diff;
    const U64 al = (((U64)t1.upper << 32) + (U64)t1.lower);
    const U64 bl = (((U64)t2.upper << 32) + (U64)t2.lower);
    if (t2.lower > t1.lower) {
        diff.lower = 0xFFFFFFFF - (t2.lower - t1.lower - 1);
        diff.upper = t1.upper - t2.upper - 1;
    } else {
        diff.lower = (U32)(al - bl);
        diff.upper = (U32)((U64)(al - bl) >> 32);
    }
    return diff;
}

}  // namespace Os
