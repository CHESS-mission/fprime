####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
# Note: using PROJECT_NAME as EXECUTABLE_NAME
####
set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/TlmChanComponentAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/TlmChanImpl.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../../Lib/mdb/pusopen_onboard_mdb.c"   # PUSOpen mission database
)

register_fprime_module()

add_definitions(-D_LINUB1804_GCC750)
include_directories(../../Lib/Includes)
find_library(PUSOPEN pusopen ../../Lib/lib)
target_link_libraries(Svc_TlmChan ${PUSOPEN})

### UTs ###
set(UT_SOURCE_FILES
  "${FPRIME_FRAMEWORK_PATH}/Svc/TlmChan/TlmChanComponentAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/TlmChanTester.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/TlmChanImplTester.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../../Lib/mdb/pusopen_onboard_mdb.c"   # PUSOpen mission database
)
register_fprime_ut()

