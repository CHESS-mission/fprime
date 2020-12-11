#include <stdio.h>

void vAssertCalled(unsigned long ulLine, const char* const pcFileName) {
    printf("ASSERT: %s : %d\n", pcFileName, (int)ulLine);
    while (1)
        ;
}

unsigned long ulGetRunTimeCounterValue(void) { return 0; }

void vConfigureTimerForRunTimeStats(void) { return; }

void vApplicationMallocFailedHook(void) {
    while (1)
        ;
}