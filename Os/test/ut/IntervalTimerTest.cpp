#include <Os/IntervalTimer.hpp>
#include <Os/Task.hpp>
#include <cstdio>

#if defined TGT_OS_TYPE_FREERTOS_SIM 
#include "FreeRTOS.h"
#include "task.h"
#endif

#define DELAY_USEC 1000

extern "C" {
    void intervalTimerTest(void);
}

void intervalTimerTest(void) {
    Os::IntervalTimer timer;
    timer.start();
    Os::Task::delay(DELAY_USEC);
    timer.stop();

    printf("Timer launched during %dus\n",timer.getDiffUsec());
    printf("Should be %dus\n", DELAY_USEC * 1000);

#if defined TGT_OS_TYPE_FREERTOS_SIM 
    printf("[FreeRTOS] Stop and relaunch program to check next test\n");
    vTaskDelete(NULL);  // To avoid assert on return from task
#endif
}
