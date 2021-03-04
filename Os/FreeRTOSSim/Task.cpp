#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/Assert.hpp>
#include <Os/Task.hpp>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

using namespace std;

//#define DEBUG_PRINT(x,...) Fw::Logger::logMsg(x,##__VA_ARGS__);
#define DEBUG_PRINT(x, ...)

namespace Os {

Task::Task()
    : m_handle(0),
      m_identifier(0),
      m_affinity(-1),
      m_started(false),
      m_suspendedOnPurpose(false) {}

Task::~Task() {
    if (Task::s_taskRegistry) {
        Task::s_taskRegistry->removeTask(this);
    }

    if(this->m_handle) {
        vTaskDelete((TaskHandle_t)this->m_handle);
        this->m_handle = 0;
    }
}

Task::TaskStatus Task::start(const Fw::StringBase& name,
                             NATIVE_INT_TYPE identifier,
                             NATIVE_INT_TYPE priority,
                             NATIVE_INT_TYPE stackSize, taskRoutine routine,
                             void* arg, NATIVE_INT_TYPE cpuAffinity) {
    Task::TaskStatus tStat = TASK_UNKNOWN_ERROR;

    this->m_name = "T_";
    this->m_name += name;
    this->m_identifier = identifier;

    TaskHandle_t tid = (TaskHandle_t)this->m_handle;

    BaseType_t stat = xTaskCreate(routine, this->m_name.toChar(), stackSize,
                                  arg, priority, &tid);

    // If a registry has been registered, register task
    if (Task::s_taskRegistry) {
        Task::s_taskRegistry->addTask(this);
    }

    switch (stat) {
        case pdPASS:
            Task::s_numTasks++;
            printf("[FreeRTOS OSAL] Task successfully created: %s (stackSize stackSize)\n", this->m_name.toChar());
            return TASK_OK;
            break;
        case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
            this->m_handle = 0; // not sure if tid has been modified
            return TASK_INVALID_PARAMS;
            break;
        default:
            this->m_handle = 0; // not sure if tid has been modified
            break;
    }
    return TASK_UNKNOWN_ERROR;
}

Task::TaskStatus Task::delay(NATIVE_UINT_TYPE milliseconds) {
    TickType_t xLastWakeTime = xTaskGetTickCount();

    if(xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(milliseconds)) == pdTRUE) {
        return TASK_OK;
    }
    return TASK_DELAY_ERROR;
}

void Task::suspend(bool onPurpose) {
    this->m_suspendedOnPurpose = onPurpose;
    vTaskSuspend((TaskHandle_t)this->m_handle);
}

void Task::resume(void) { vTaskResume((TaskHandle_t)this->m_handle); }

bool Task::isSuspended(void) {
    return (eTaskGetState((TaskHandle_t)this->m_handle) == eSuspended);
}

Task::TaskStatus Task::join(void **value_ptr) {
    vTaskDelete(NULL);  // To avoid assert on return from task
    return TASK_JOIN_ERROR;
}

}  // namespace Os
