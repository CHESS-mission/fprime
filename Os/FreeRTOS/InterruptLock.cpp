#include <Os/InterruptLock.hpp>
#include <Os/Mutex.hpp>

namespace Os {

InterruptLock::InterruptLock() : m_key(0) {}

InterruptLock::~InterruptLock() {}

void InterruptLock::lock(void) {}

void InterruptLock::unLock(void) {}

}  // namespace Os
