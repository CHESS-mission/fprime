/**
 * FreeRTOS/IntervalTimer.cpp:
 */
#include <errno.h>
#include <string.h>
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

void IntervalTimer::getRawTime(RawTime& time) {
    TickType_t xTime1;

    /* Get the time the function started. */

    xTime1 = 0; // @todo xTaskGetTickCount();
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

}  // namespace Os
