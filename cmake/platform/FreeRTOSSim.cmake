####
# FreeRTOS.cmake: CMake to compile F' on Posix/Linux Simulator for FreeRTOS
####
add_definitions(-DTGT_OS_TYPE_FREERTOS_SIM)

# In our case the Posix/Linux simulator from FreeRTOS will obviously use Posix
# but current definition concern F' compilation. For F', it shouldn't use Posix
# but FreeRTOS only
set(FPRIME_USE_POSIX OFF)

# Relative FreeRTOS from current CMake source directory (05_FS/App/)
set(FREERTOS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../FreeRTOS/FreeRTOS")

include_directories(
    .
    ${FREERTOS_DIR}/Source/include
    ${FREERTOS_DIR}/Source/portable/ThirdParty/GCC/Posix
    ${FREERTOS_DIR}/Source/portable/ThirdParty/GCC/Posix/utils
    ${FREERTOS_DIR}/Demo/Common/include
    ${FREERTOS_DIR}/Demo/Posix_GCC
)

# Generic Code
FILE(GLOB FREERTOS_SOURCES
    ${FREERTOS_DIR}/Source/*.c
)

# Memory manager (use malloc() / free())
list(APPEND FREERTOS_SOURCES "${FREERTOS_DIR}/Source/portable/MemMang/heap_3.c")

# Plaform Port Specific Code
list(APPEND FREERTOS_SOURCES "${FREERTOS_DIR}/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c")
list(APPEND FREERTOS_SOURCES "${FREERTOS_DIR}/Source/portable/ThirdParty/GCC/Posix/port.c")

# C flags
add_definitions(-DprojCOVERAGE_TEST=0)
add_definitions(-D_WINDOWS_)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3 -O0")

# Linker flags
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ggdb3 -O0")

add_library(freertos STATIC ${FREERTOS_SOURCES})
#target_link_libraries(App_exe freertos)

include_directories(SYSTEM "${FPRIME_FRAMEWORK_PATH}/Fw/Types/FreeRTOSSim")
