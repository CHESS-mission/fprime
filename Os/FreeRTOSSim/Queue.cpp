// ======================================================================
// \title  Queue.cpp
// \author jonathanmichel
// \brief  Queue implementation using FreeRTOS.
// ======================================================================
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <Fw/Types/Assert.hpp>
#include <Os/Pthreads/BufferQueue.hpp>
#include <Os/Queue.hpp>

#include "FreeRTOS.h"
#include "queue.h"

namespace Os {

Queue::Queue() : m_handle(0) {}

Queue::QueueStatus Queue::createInternal(const Fw::StringBase &name,
                                         NATIVE_INT_TYPE depth,
                                         NATIVE_INT_TYPE msgSize) {
    QueueHandle_t queueHandle;

    this->m_name = "/Q_";
    this->m_name += name;

    queueHandle = xQueueCreate(depth, msgSize + sizeof(msgSize));

    if (queueHandle != NULL) {
        this->m_handle = (POINTER_CAST)queueHandle;
        this->depth = depth;
        this->msgSize = msgSize;

        Queue::s_numQueues++;

        this->msg_buffer = (U8 *)pvPortMalloc(msgSize + sizeof(msgSize));

        return QUEUE_OK;
    }

    return QUEUE_UNINITIALIZED;
}

Queue::~Queue() {
    if (this->m_handle) {
        vQueueDelete((QueueHandle_t)this->m_handle);
    }

    if (this->msg_buffer) {
        vPortFree(this->msg_buffer);
    }
}

Queue::QueueStatus Queue::send(const U8 *buffer, NATIVE_INT_TYPE size,
                               NATIVE_INT_TYPE priority, QueueBlocking block) {
    QueueHandle_t queueHandle = (QueueHandle_t)this->m_handle;

    if (queueHandle == NULL) {
        printf("QUEUE_UNINITIALIZED");
        return QUEUE_UNINITIALIZED;
    }

    if (buffer == NULL) {
        printf("QUEUE_EMPTY_BUFFER");
        return QUEUE_EMPTY_BUFFER;
    }

    /*/ @todo Check how size is used
    // QUEUE_SIZE_MISMATCH : attempted to send or receive with buffer too large,
    // too small
    if (size != getMsgSize()) {
        printf("QUEUE_SIZE_MISMATCH");
        return QUEUE_SIZE_MISMATCH;
    }
    //*/

    this->msg_buffer[0] = size;
    memcpy(this->msg_buffer + sizeof(size), buffer, size);

    if (block == QUEUE_NONBLOCKING) {
        if (xQueueSendToBack(queueHandle, (void *)buffer, (TickType_t)0) ==
            errQUEUE_FULL) {
            printf("QUEUE_FULL");

            return QUEUE_FULL;
        }
    } else {
        if (xQueueSendToBack(queueHandle, (void *)buffer,
                             (TickType_t)portMAX_DELAY) != pdTRUE) {
            printf("QUEUE_UNKNOWN_ERROR");
            // Given the fact it's a blocking call, xQueueSendToBack should only
            // block or return pdTRUE
            return QUEUE_SEND_ERROR;
        }
    }

    return QUEUE_OK;
}

Queue::QueueStatus Queue::receive(U8 *buffer, NATIVE_INT_TYPE capacity,
                                  NATIVE_INT_TYPE &actualSize,
                                  NATIVE_INT_TYPE &priority,
                                  QueueBlocking block) {
    NATIVE_INT_TYPE actualSizeFromSend = 0;
    QueueHandle_t queueHandle = (QueueHandle_t)this->m_handle;

    if (queueHandle == NULL) {
        return QUEUE_UNINITIALIZED;
    }

    if (buffer == NULL) {
        return QUEUE_EMPTY_BUFFER;
    }

    // @todo Check how size is used
    // Maybe capacity has to be checked
    // QUEUE_SIZE_MISMATCH : attempted to send or receive with buffer too large,
    // too small
    if (actualSize != getMsgSize()) {
        return QUEUE_SIZE_MISMATCH;
    }

    if (block == QUEUE_NONBLOCKING) {
        if (xQueueReceive(queueHandle, (void *)buffer, (TickType_t)0) !=
            pdTRUE) {
            return QUEUE_NO_MORE_MSGS;
        }
    } else {  // QUEUE_BLOCKING
        if (xQueueReceive(queueHandle, (void *)buffer,
                          (TickType_t)portMAX_DELAY) != pdTRUE) {
            // Given the fact it's a blocking call, xQueueReceive should only
            // block or return pdTRUE
            return QUEUE_RECEIVE_ERROR;
        }
    }

    actualSizeFromSend = this->msg_buffer[0];
    memcpy(buffer, this->msg_buffer + sizeof(actualSizeFromSend), actualSizeFromSend);

    return QUEUE_OK;
}

NATIVE_INT_TYPE Queue::getNumMsgs(void) const {
    return uxQueueMessagesWaiting((QueueHandle_t)this->m_handle);
}

NATIVE_INT_TYPE Queue::getMaxMsgs(void) const { return 0; }

#if TGT_OS_TYPE_FREERTOS_SIM

NATIVE_INT_TYPE Queue::getQueueSize(void) const { return this->depth; }

NATIVE_INT_TYPE Queue::getMsgSize(void) const { return this->msgSize; }

#endif

}  // namespace Os
