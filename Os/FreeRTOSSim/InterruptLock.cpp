#include <Os/InterruptLock.hpp>
#include <Os/Mutex.hpp>

namespace Os {

STATIC Os::Mutex mutex;

InterruptLock::InterruptLock() : m_key(0) {}

InterruptLock::~InterruptLock() {}

void InterruptLock::lock(void) { mutex.lock(); }

void InterruptLock::unLock(void) { mutex.unLock(); }

}  // namespace Os
