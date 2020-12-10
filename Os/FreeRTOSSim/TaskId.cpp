#include <Os/TaskId.hpp>

#include "FreeRTOS.h"

namespace Os {
TaskId::TaskId(void) { id = 0 }   // @todo pxCurrentTCB->uxTCBNumber 

TaskId::~TaskId(void) {}

bool TaskId::operator==(const TaskId& T) const {
    return (id == T.id);
}

bool TaskId::operator!=(const TaskId& T) const {
    return (id != T.id);
}

TaskIdRepr TaskId::getRepr(void) const { return this->id; }

}  // namespace Os
