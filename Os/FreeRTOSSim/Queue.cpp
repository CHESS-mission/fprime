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

using namespace std;

namespace Os {

Queue::Queue() : m_handle(-1) {}

Queue::QueueStatus Queue::createInternal(const Fw::StringBase &name,
                                         NATIVE_INT_TYPE depth,
                                         NATIVE_INT_TYPE msgSize) {
    QueueHandle_t queueHandle;

    this->m_name = "/Q_";
    this->m_name += name;

    //printf("---[%s] Create queue: depth %u, msgSize %u\n", this->m_name.toChar(), depth,
    //       msgSize);

    queueHandle = xQueueCreate(depth, msgSize + sizeof(msgSize));

    if (queueHandle != NULL) {
        this->m_handle = (POINTER_CAST)queueHandle;
        this->depth = depth;
        this->msgSize = msgSize;

        Queue::s_numQueues++;

        this->msg_buffer = (U8 *)pvPortMalloc(msgSize + sizeof(msgSize));
        if (this->msg_buffer == NULL) {
            return QUEUE_UNINITIALIZED;
        }
        printf("[FreeRTOS OSAL] Queue successfully created: %s (depth %u, msgSize %u)\n",
         this->m_name.toChar(), depth, msgSize);
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
        //printf("QUEUE_UNINITIALIZED\n");
        return QUEUE_UNINITIALIZED;
    }

    if (buffer == NULL) {
        //printf("QUEUE_EMPTY_BUFFER\n");
        return QUEUE_EMPTY_BUFFER;
    }

    // The message queue capacity is too small
    if (size > this->getMsgSize()) {
        //printf("QUEUE_SIZE_MISMATCH\n");
        return QUEUE_SIZE_MISMATCH;
    }

    // FreeRTOS doesn't return message size on reading. We need to save it
    // before the message itself to be able to return it to the user
    this->msg_buffer[0] = size;
    memcpy(this->msg_buffer + sizeof(size), buffer, size);

    if (block == QUEUE_NONBLOCKING) {
        if (xQueueSendToBack(queueHandle, (void *)this->msg_buffer, (TickType_t)0) ==
            errQUEUE_FULL) {
            //printf("QUEUE_FULL\n");
            return QUEUE_FULL;
        }
    } else {
        if (xQueueSendToBack(queueHandle, (void *)this->msg_buffer,
                             (TickType_t)portMAX_DELAY) != pdTRUE) {
            //printf("QUEUE_UNKNOWN_ERROR\n");
            // Given the fact it's a blocking call, xQueueSendToBack should only
            // block or return pdTRUE
            return QUEUE_SEND_ERROR;
        }
    }

    //printf("---[%s] Data sent: buffer %p, size %u in %sBLOCKING mode\n",
    //       this->m_name.toChar(), (void *)this->msg_buffer, size, block == QUEUE_BLOCKING ? "" : "NON-");

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

    //printf("---[%s] Try to receive data: buffer %p, size %u in %sBLOCKING mode\n",
    //       this->m_name.toChar(), (void *)buffer, actualSize, block == QUEUE_BLOCKING ? "" : "NON-");

    if (block == QUEUE_BLOCKING) {
        if (xQueueReceive(queueHandle, (void *)this->msg_buffer,
                          (TickType_t)portMAX_DELAY) != pdTRUE) {
            // Given the fact it's a blocking call, xQueueReceive should only
            // block or return pdTRUE
            return QUEUE_RECEIVE_ERROR;
        }

    } else {  // QUEUE_NONBLOCKING
        if (xQueueReceive(queueHandle, (void *)this->msg_buffer, (TickType_t)0) !=
            pdTRUE) {
            //printf("------[%s] No more messages\n",
            //    this->m_name.toChar());
            return QUEUE_NO_MORE_MSGS;
        }
    }

    // Get actual message size saved on sent and return
    // it to the user
    actualSize = this->msg_buffer[0];

    if (actualSize > this->getMsgSize()) {
        return QUEUE_SIZE_MISMATCH;
    }

    memcpy(buffer, this->msg_buffer + sizeof(actualSize), actualSize);
    priority = 0;   // FiFo queue, no priority handling. Return 0 for UT

    //printf("------[%s] Data received: size %u\n",
    //       this->m_name.toChar(), actualSize);

    return QUEUE_OK;
}

NATIVE_INT_TYPE Queue::getNumMsgs(void) const {
    return uxQueueMessagesWaiting((QueueHandle_t)this->m_handle);
}

NATIVE_INT_TYPE Queue::getMaxMsgs(void) const { return 0; }

NATIVE_INT_TYPE Queue::getQueueSize(void) const { return this->depth; }

NATIVE_INT_TYPE Queue::getMsgSize(void) const { return this->msgSize; }

}  // namespace Os
