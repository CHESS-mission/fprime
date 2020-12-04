#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/Assert.hpp>
#include <Os/Task.hpp>

//#define DEBUG_PRINT(x,...) Fw::Logger::logMsg(x,##__VA_ARGS__);
#define DEBUG_PRINT(x, ...)

namespace Os {

Task::Task()
    : m_handle(0),
      m_identifier(0),
      m_affinity(-1),
      m_started(false),
      m_suspendedOnPurpose(false) {}

Task::TaskStatus Task::start(const Fw::StringBase& name,
                             NATIVE_INT_TYPE identifier,
                             NATIVE_INT_TYPE priority,
                             NATIVE_INT_TYPE stackSize, taskRoutine routine,
                             void* arg, NATIVE_INT_TYPE cpuAffinity) {
    return TASK_OK;
}

Task::~Task() {}

Task::TaskStatus Task::delay(NATIVE_UINT_TYPE milliseconds) {
    return TASK_OK;  // for coverage analysis
}

void Task::suspend(bool onPurpose) { FW_ASSERT(0); }

void Task::resume(void) { FW_ASSERT(0); }

bool Task::isSuspended(void) {
    FW_ASSERT(0);
    return false;
}

TaskId Task::getOsIdentifier(void) {
    TaskId T;
    return T;
}

Task::TaskStatus Task::join(void** value_ptr) { return TASK_OK; }

}  // namespace Os
