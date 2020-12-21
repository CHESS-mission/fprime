/**
 * FreeRTOS/IntervalTimer.cpp:
 */
#include <errno.h>
#include <string.h>
#include <time.h>

#include <Fw/Types/Assert.hpp>
#include <Os/IntervalTimer.hpp>

#include "FreeRTOS.h"
#include "task.h"

namespace Os {

void IntervalTimer::getRawTime(RawTime& time) {
    TickType_t xTime1;

    xTime1 = xTaskGetTickCount();
    time.lower = 1000 * xTime1; // pdMS_TO_TICKS
    time.upper = 0;
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

}  // namespace Os
