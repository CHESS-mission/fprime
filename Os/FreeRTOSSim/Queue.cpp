// ======================================================================
// \title  Queue.cpp
// \author jonathanmichel
// \brief  Queue implementation using FreeRTOS.
// ======================================================================

#include <errno.h>
#include <pthread.h>
#include <stdio.h>

#include <Fw/Types/Assert.hpp>
#include <Os/Pthreads/BufferQueue.hpp>
#include <Os/Queue.hpp>

#include "FreeRTOS.h"

namespace Os {

Queue::Queue() : m_handle(-1) {}

Queue::QueueStatus Queue::create(const Fw::StringBase &name,
                                 NATIVE_INT_TYPE depth,
                                 NATIVE_INT_TYPE msgSize) {
    QueueHandle_t queueHandle;

    this->m_name = "/Q_";
    this->m_name += name;

#ifdef configUSE_TRACE_FACILITY
    vTraceSetQueueName(queueHandle, this->m_name.toChar());
#endif

    queueHandle = xQueueCreate(depth, msgSize + sizeof(msgSize));

    if (NULL != queueHandle) {
        this->m_handle = (POINTER_CAST)queueHandle;
        this->depth = depth;
        this->msgSize = msgSize;

        Queue::s_numQueues++;

        msg_buffer = (U8 *)pvPortMalloc(msgSize + sizeof(msgSize));

        return QUEUE_OK;
    }

    return QUEUE_UNINITIALIZED;
}

Queue::~Queue() {
    if (this->m_handle) {
        vQueueDelete((QueueHandle_t)this->m_handle);
    }

    if (msg_buffer) {
        vPortFree(msg_buffer);
    }
}

Queue::QueueStatus Queue::send(const U8 *buffer, NATIVE_INT_TYPE size,
                               NATIVE_INT_TYPE priority, QueueBlocking block) {
    QueueHandle_t queueHandle = (QueueHandle_t)this->m_handle;

    if (queueHandle == NULL) {
        return QUEUE_UNINITIALIZED;
    }

    if (buffer == NULL) {
        return QUEUE_EMPTY_BUFFER;
    }

    msg_buffer[0] = size;
    memcpy(msg_buffer + sizeof(size), buffer, size);

    if (size != getMsgSize()) {
        return QUEUE_SIZE_MISMATCH;
    }

    if (block == QUEUE_NONBLOCKING) {
        if (xQueueSendToBack(queueHandle, (void *)msg_buffer, (TickType_t)0) ==
            errQUEUE_FULL) {
            return QUEUE_FULL;
        }
    } else {
        if (xQueueSendToBack(queueHandle, (void *)msg_buffer,
                             (TickType_t)portMAX_DELAY) != pdPASS) {
            return QUEUE_UNKNOWN_ERROR;
        }
    }

    return QUEUE_OK;
}

Queue::QueueStatus Queue::receive(U8 *buffer, NATIVE_INT_TYPE capacity,
                                  NATIVE_INT_TYPE &actualSize,
                                  NATIVE_INT_TYPE &priority,
                                  QueueBlocking block) {
    QueueHandle_t queueHandle = (QueueHandle_t)this->m_handle;

    if (queueHandle == NULL) {
        return QUEUE_UNINITIALIZED;
    }

    if (buffer == NULL) {
        return QUEUE_EMPTY_BUFFER;
    }

    if (size != getMsgSize()) {
        return QUEUE_SIZE_MISMATCH;
    }

    if (block == QUEUE_NONBLOCKING) {
        if (xQueueReceive(queueHandle, (void *)msg_buffer, (TickType_t)0) ==
            errQUEUE_EMPTY) {
            return QUEUE_NO_MORE_MSGS;
        }
    } else {
        if (xQueueReceive(queueHandle, (void *)msg_buffer,
                          (TickType_t)portMAX_DELAY) == errQUEUE_EMPTY) {
            return QUEUE_NO_MORE_MSGS;
        }
    }

    actualSize = msg_buffer[0];
    memcpy(buffer, msg_buffer + sizeof(actualSize), actualSize);

    return QUEUE_OK;
}

NATIVE_INT_TYPE Queue::getNumMsgs(void) const {
    return uxQueueMessagesWaiting((QueueHandle_t)this->m_handle);
}

NATIVE_INT_TYPE Queue::getMaxMsgs(void) const { return 0; }

NATIVE_INT_TYPE Queue::getQueueSize(void) const { return this->depth; }

NATIVE_INT_TYPE Queue::getMsgSize(void) const { return this->msgSize; }

}  // namespace Os
