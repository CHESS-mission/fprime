#include <Fw/Types/Assert.hpp>
#include <Os/Mutex.hpp>

#include "FreeRTOS.h"

namespace Os {

Mutex::Mutex(void) {
    SemaphoreHandle_t xSemaphore;

    xSemaphore = xSemaphoreCreateMutex();
    if (xSemaphore == NULL) {
        FW_ASSERT(xSemaphore == 0, (U32)xSemaphore);
    }
    this->m_handle = (POINTER_CAST)xSemaphore;
}

Mutex::~Mutex(void) {
    if (this->m_handle != NULL) {
        vSemaphoreDelete((SemaphoreHandle_t)this->m_handle);
    }
}

void Mutex::lock(void) {
    SemaphoreHandle_t xSemaphore = (SemaphoreHandle_t)this->m_handle;
    if (xSemaphore) {
        if (xSemaphoreTake(xSemaphore, 10) != pdPASS) {
            FW_ASSERT(false);
        }
    }
}

void Mutex::unLock(void) {
    SemaphoreHandle_t xSemaphore = (SemaphoreHandle_t)this->m_handle;
    if (xSemaphore) {
        if (xSemaphoreGive(xSemaphore) != pdPASS) {
            FW_ASSERT(false);
        }
    }

}  // namespace Os