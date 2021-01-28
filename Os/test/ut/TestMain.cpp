#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

/*
 * OSAL tests have to be launched manually for FreeRTOSSim
 *
 * fprime/Os ❯ fprime-util build --ut -j 8 // to build OSAL tests 
 * fprime/Os ❯ ../build-fprime-automatic-native-ut/bin/FreeRTOSSim/Os_ut_exe [testNb] // to run them
 *
 * [testNb] in fprime/Os/test/ut/TestMain.cpp
 * Current results : 
 *   Test 6 qtest_concurrent(): Seg fault
 *   Other tests passed
 * 
 * For Linux
 * fprime/Os ❯ fu build --ut -j 8
 * fprime/OS ❯ fu check
*/

#if defined TGT_OS_TYPE_LINUX || TGT_OS_TYPE_DARWIN
extern "C" {
  void startTestTask(int iters);
  void qtest_block_receive(void);
  void qtest_nonblock_receive(void);
  void qtest_nonblock_send(void);
  void qtest_block_send(void);
  void qtest_performance(void);
  void qtest_concurrent(void);
  void intervalTimerTest(void);
  void fileSystemTest(void);
  void validateFileTest(void);
}
#endif

#if defined TGT_OS_TYPE_FREERTOS_SIM
#include "FreeRTOS.h"
#include "task.h"

extern "C" {
  void startTestTask(int iters);
  void qtest_block_receive(void*);
  void qtest_nonblock_receive(void*);
  void qtest_nonblock_send(void*);
  void qtest_block_send(void*);
  void qtest_performance(void*);
  void qtest_concurrent(void*);
  void intervalTimerTest(void*);
}
#endif


#if defined TGT_OS_TYPE_LINUX || TGT_OS_TYPE_DARWIN
void run_test(int test_num)
{
  switch(test_num) {
    case 0:
      startTestTask(10);
      sleep(15);
      break;
    case 1:
      qtest_block_receive();
      break;
    case 2:
      qtest_nonblock_receive();
      break;
    case 3:
      qtest_nonblock_send();
      break;
    case 4:
      qtest_block_send();
      break;
    case 5:
      qtest_performance();
      /*******************RESULTS****************************
      Running OS ut tests
      -----------------------------
      ---- performance test -------
      -----------------------------
      Testing shallow queue...
      Time: 1.041s (1.041us per)
      Testing deep queue...
      Time: 1.400s (1.400us per)
      Test complete.
      -----------------------------
      -----------------------------
      *******************************************************/
      break;
    case 6:
      qtest_concurrent();
      break;
    case 7:
      intervalTimerTest();
      break;
    case 8: 
      fileSystemTest();
      break;
    case 9:
      //validateFileTest();
      printf("validateFileTest() not implemented on Linux\n");
      break;
    default:
      fprintf(stderr, "Invalid test number: %d\n", test_num);
      break;
  }
}
#endif

#if defined TGT_OS_TYPE_FREERTOS_SIM 
// When running UT on FreeRTOS, tests have to be run independantly
// It is due to scheduller launched with vTaskStartScheduler() that 
// never returns
void run_test(int test_num)
{
  printf("Running test %u\n", test_num);
  switch(test_num) {
    case 0:
      startTestTask(5);
      vTaskStartScheduler();
      break;
    case 1:
      xTaskCreate(qtest_block_receive, (const char*) "qtest_block_receive", 2048, 0, 1, 0);
      vTaskStartScheduler();
      break;
    case 2:
      xTaskCreate(qtest_nonblock_receive, (const char*) "qtest_nonblock_receive", 2048, 0, 1, 0);
      vTaskStartScheduler();
      break;
    case 3:
      xTaskCreate(qtest_nonblock_send, (const char*) "qtest_nonblock_send", 2048, 0, 1, 0);
      vTaskStartScheduler();
      break;
    case 4:
      xTaskCreate(qtest_block_send, (const char*) "qtest_block_send", 2048, 0, 1, 0);
      vTaskStartScheduler();
      break;
    case 5:
      xTaskCreate(qtest_performance, (const char*) "qtest_performance", 2048, 0, 1, 0);
      vTaskStartScheduler();
    /*******************RESULTS****************************
    Running OS ut tests
    Running test 5
    -----------------------------
    ---- performance test -------
    -----------------------------
    ---[/Q_TestQ] Create queue: depth 10, msgSize 100
    Testing shallow queue...
    Time: 56.741s (56.741us per)
    Testing deep queue...
    Time: 58.265s (58.265us per)
    Test complete.
    -----------------------------
    -----------------------------
    [FreeRTOS] Stop and relaunch program to check next test
    *******************************************************/
      break;
    case 6:
      xTaskCreate(qtest_concurrent, (const char*) "qtest_concurrent", 2048, 0, 1, 0);
      vTaskStartScheduler();
      break;
    case 7:
      xTaskCreate(intervalTimerTest, (const char*) "intervalTimerTest", 2048, 0, 1, 0);
      vTaskStartScheduler();
      break;
    case 8:
      //fileSystemTest(); -- Unimplemented fs in FreeRTOS
      printf("No file system on FreeRTOS\n");
      break;
    case 9:
      //validateFileTest(); -- Unimplemented fs in FreeRTOS
      printf("No file system on FreeRTOS\n");
      break;
    default:
      fprintf(stderr, "Invalid test number: %d\n", test_num);
      break;
  }
}
#endif

int main(int argc, char* argv[]) {
  int i;
  printf("Running OS ut tests\n");
  if( argc > 1 ) {
    int test_num = atoi(argv[1]);
    run_test(test_num);
  } else {
    for(i = 0; i < 10; i++) {
      run_test(i);
    }
  }
  return 0;
}
