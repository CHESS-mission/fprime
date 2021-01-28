####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
####

# Basic module dependencies
set(MOD_DEPS
  "${CMAKE_THREAD_LIBS_INIT}" 
  Fw/Cfg
  Fw/Types
  Fw/Logger
  Utils/Hash
)

# Basic source files used in every OSAL layer.  Contains common code and helpers.
set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/IntervalTimerCommon.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/TaskString.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/TaskCommon.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/QueueCommon.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/QueueString.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/IPCQueueCommon.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/SimpleQueueRegistry.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/MemCommon.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/ValidateFileCommon.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/ValidatedFile.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/FileCommon.cpp"
)
# Check for default loger
if(NOT FPRIME_DISABLE_DEFAULT_LOGGER)
  list(APPEND SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/LogDefault.cpp"
  )
endif()

# FreeRTOS OSAL implementation
if(${PLATFORM} STREQUAL "FreeRTOSSim")
  list(APPEND SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/FreeRTOSSim/assert.c"
    "${CMAKE_CURRENT_LIST_DIR}/FreeRTOSSim/File.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/FreeRTOSSim/FileSystem.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/FreeRTOSSim/InterruptLock.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/FreeRTOSSim/IntervalTimer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/FreeRTOSSim/Mutex.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/FreeRTOSSim/Queue.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/FreeRTOSSim/Task.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/FreeRTOSSim/WatchdogTimer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/LogPrintf.cpp"
  )
elseif(FPRIME_USE_POSIX)  # Posix systems typically share these
  list(APPEND SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/LogPrintf.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Pthreads/Queue.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Pthreads/BufferQueueCommon.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Pthreads/PriorityBufferQueue.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Pthreads/MaxHeap/MaxHeap.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Linux/File.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Posix/Task.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Linux/InterruptLock.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Linux/WatchdogTimer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Posix/IntervalTimer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Posix/Mutex.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Linux/FileSystem.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Posix/TaskId.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Linux/Directory.cpp"
  )
endif()

# Darwin IPC queue implementation
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  list(APPEND SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/MacOs/IPCQueueStub.cpp"
  )
elseif(${PLATFORM} STREQUAL "FreeRTOSSim")  # avoid next condition entry
  message("FreeRTOSSim Queue implementation")
  # FreeRTOS Queue implementation include above
# Linux IPC queues implementation
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "arm-linux-gnueabihf")
  list(APPEND SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/Posix/IPCQueue.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Posix/LocklessQueue.cpp"
  )
  # Shared libraries need an -rt dependency for mq libs
  if (BUILD_SHARED_LIBS)
      list(APPEND MOD_DEPS "-lrt")
  endif()
endif()

# If baremetal scheduler is set, remove the previouse task files and add in the Baremetal variant
if (BAREMETAL_SCHEDULER)
    add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Baremetal/TaskRunner/")
    list(FILTER SOURCE_FILES EXCLUDE REGEX "Task\.cpp")
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/Baremetal/Task.cpp")
endif()
register_fprime_module()

# After register_fprime_module() 
if(${PLATFORM} STREQUAL "FreeRTOSSim")
  # Has to be done after register_fprime_module()
  find_library(freertos "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
  target_link_libraries(Os freertos) 
endif()

# Add stubs directory for testing builds
if (CMAKE_BUILD_TYPE STREQUAL "TESTING")
    add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Stubs/")
endif()

### UTS ### Note: 3 separate UTs registered here.
if(${PLATFORM} STREQUAL "FreeRTOSSim") 
  set(UT_SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/TestMain.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/OsQueueTest.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/IntervalTimerTest.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/OsTaskTest.cpp"
    #File system being not implemented on FreeRTOS, there is no corresponding UT 
    #"${CMAKE_CURRENT_LIST_DIR}/test/ut/OsValidateFileTest.cpp"
    #"${CMAKE_CURRENT_LIST_DIR}/test/ut/OsFileSystemTest.cpp"
  )
  register_fprime_ut()  
else()
  set(UT_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/OsQueueTest.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/TestMain.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/IntervalTimerTest.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/OsTaskTest.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/OsValidateFileTest.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/OsFileSystemTest.cpp"
  )
  ## TODO: **BROKEN UT**, validation of File fails (F' bug)
  register_fprime_ut()  

  # Avoid Pthreads tests when platform is FreeRTOSSim
  # Second UT Pthreads
  set(UT_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/Pthreads/test/ut/BufferQueueTest.cpp"
  )
  register_fprime_ut("Os_pthreads")

  # Third  UT Pthrads MAX Heap
  set(UT_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/Pthreads/MaxHeap/test/ut/MaxHeapTest.cpp"
  )
  register_fprime_ut("Os_pthreads_max_heap")
endif()
