// ======================================================================
// \title  Queue.cpp
// \author dinkel
// \brief  Queue implementation using the pthread library. This is NOT
//         an IPC queue. It is meant to be used between threads within
//         the same address space.
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================

#include <errno.h>
#include <pthread.h>
#include <stdio.h>

#include <Fw/Types/Assert.hpp>
#include <Os/Pthreads/BufferQueue.hpp>
#include <Os/Queue.hpp>

namespace Os {

// A helper class which stores variables for the queue handle.
// The queue itself, a pthread condition variable, and pthread
// mutex are contained within this container class.
class QueueHandle {
   public:
    QueueHandle() {}

    ~QueueHandle() {}

    bool create(NATIVE_INT_TYPE depth, NATIVE_INT_TYPE msgSize) {
        return queue.create(depth, msgSize);
    }
};

Queue::Queue() : m_handle((POINTER_CAST)NULL) {}

Queue::QueueStatus Queue::createInternal(const Fw::StringBase& name,
                                         NATIVE_INT_TYPE depth,
                                         NATIVE_INT_TYPE msgSize) {
    return QUEUE_OK;
}

Queue::~Queue() {}

Queue::QueueStatus sendNonBlock(QueueHandle* queueHandle, const U8* buffer,
                                NATIVE_INT_TYPE size,
                                NATIVE_INT_TYPE priority) {
    return 0;
}

Queue::QueueStatus sendBlock(QueueHandle* queueHandle, const U8* buffer,
                             NATIVE_INT_TYPE size, NATIVE_INT_TYPE priority) {
    return Queue::QUEUE_OK;
}

Queue::QueueStatus Queue::send(const U8* buffer, NATIVE_INT_TYPE size,
                               NATIVE_INT_TYPE priority, QueueBlocking block) {
    return Queue::QUEUE_OK;
}

Queue::QueueStatus receiveNonBlock(QueueHandle* queueHandle, U8* buffer,
                                   NATIVE_INT_TYPE capacity,
                                   NATIVE_INT_TYPE& actualSize,
                                   NATIVE_INT_TYPE& priority) {
    return Queue::QUEUE_OK;
}

Queue::QueueStatus receiveBlock(QueueHandle* queueHandle, U8* buffer,
                                NATIVE_INT_TYPE capacity,
                                NATIVE_INT_TYPE& actualSize,
                                NATIVE_INT_TYPE& priority) {
    return Queue::QUEUE_OK;
}

Queue::QueueStatus Queue::receive(U8* buffer, NATIVE_INT_TYPE capacity,
                                  NATIVE_INT_TYPE& actualSize,
                                  NATIVE_INT_TYPE& priority,
                                  QueueBlocking block) {
    return Queue::QUEUE_OK;
}

NATIVE_INT_TYPE Queue::getNumMsgs(void) const { return 0; }

NATIVE_INT_TYPE Queue::getMaxMsgs(void) const { return 0; }

NATIVE_INT_TYPE Queue::getQueueSize(void) const { return 0; }

NATIVE_INT_TYPE Queue::getMsgSize(void) const { return 0; }

}  // namespace Os
