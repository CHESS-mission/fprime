add_definitions(-DTGT_OS_TYPE_RTEMS)

include_directories(SYSTEM
  ${FREERTOS_INCLUDE_DIRS}
  ${FREERTOS_BSP_INCLUDE_DIRS}
#  ${FREERTOS_SOURCE_DIR}/testsuites/support/include
)

add_compile_options(${FREERTOS_C_FLAGS})

add_link_options(${FREERTOS_LINK_FLAGS})

#add_compile_definitions(
#  __rtems__
#  _POSIX_THREADS
#)