#include <FpConfig.hpp>
#include <Fw/Types/Assert.hpp>
#include <Fw/Types/BasicTypes.hpp>
#include <Os/File.hpp>

namespace Os {

File::File() : m_fd(0), m_mode(OPEN_NO_MODE), m_lastError(0) {}

File::~File() {}

File::Status File::open(const char* fileName, File::Mode mode) {
    return DOESNT_EXIST;
}

File::Status File::seek(NATIVE_INT_TYPE offset, bool absolute) {
    return NOT_OPENED;
}

File::Status File::read(void* buffer, NATIVE_INT_TYPE& size, bool waitForFull) {
    return NOT_OPENED;
}

File::Status File::write(const void* buffer, NATIVE_INT_TYPE& size,
                         bool waitForDone) {
    return NOT_OPENED;
}

void File::close(void) {}

NATIVE_INT_TYPE File::getLastError(void) { return -1; }

const char* File::getLastErrorString(void) { return ""; }

}  // namespace Os