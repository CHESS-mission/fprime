####
# FreeRTOS.cmake:
####
add_definitions(-DTGT_OS_TYPE_FREERTOS)

# Find an appropriate thread library
message(STATUS "Requiring thread library")
FIND_PACKAGE ( Threads REQUIRED )
set(FPRIME_USE_POSIX ON)

# FreeRTOS specific flags: C, and C++ settings
set(CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS} ${DARWIN_COMMON} -std=c99 -pedantic -Werror-implicit-function-declaration -Wstrict-prototypes"
)
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} ${DARWIN_COMMON} -std=c++11"
)
# Update for FreeRTOS ?
# Add linux include path which is compatable with Darwin for StandardTypes.hpp
# include_directories(SYSTEM "${FPRIME_FRAMEWORK_PATH}/Fw/Types/Linux")
