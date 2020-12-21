#include <Os/Task.hpp>
#include <stdio.h>
#include <Fw/Types/EightyCharString.hpp>

#if defined TGT_OS_TYPE_FREERTOS_SIM 
#include "FreeRTOS.h"
#include "task.h"
#endif

extern "C" {
    void startTestTask(int iters);
}

Os::Task* testTask = 0;

void someTask(void* ptr) {
    
    long iters = (long) ptr;
    
    while (iters--) {
        Os::Task::delay(1000);
        printf("Tick %ld!\n",iters);
    }

    printf("Passed.\n");
    printf("Test complete.\n");
    printf("-----------------------------\n");
    printf("-----------------------------\n");

#if defined TGT_OS_TYPE_FREERTOS_SIM 
    printf("[FreeRTOS] Stop and relaunch program to check next test\n");
    vTaskDelete(NULL);  // To avoid assert on return from task
#endif
}

void startTestTask(int iters) {
    long localIter = iters;
    testTask = new Os::Task();
    Fw::EightyCharString name("ATestTask");
    Os::Task::TaskStatus stat = testTask->start(name,12,100,10*1024,someTask,(void*) localIter);
    if (stat != Os::Task::TASK_OK) {
    	printf("Couldn't start task: %d\n",stat);
    }
}
