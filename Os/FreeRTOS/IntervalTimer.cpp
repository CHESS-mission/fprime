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

namespace Os {

void IntervalTimer::getRawTime(RawTime& time) {}

// Adapted from:
// http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html should be
// t1In - t2In
U32 IntervalTimer::getDiffUsec(const RawTime& t1In, const RawTime& t2In) {
    return 0;
}

}  // namespace Os
