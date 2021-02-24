####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
# Note: using PROJECT_NAME as EXECUTABLE_NAME
####
SET(PUSOPEN_PATH ${FPRIME_FRAMEWORK_PATH}/Lib)
SET(MDB_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../gs/mdb)

set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/TlmChanComponentAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/TlmChanImpl.cpp"
  "${MDB_PATH}/pusopen_onboard_mdb.c"                    # PUSOpen mission database
)

register_fprime_module()

add_definitions(-D_LINUB1804_GCC750)
include_directories(${MDB_PATH}) # hk_param.h
include_directories(${PUSOPEN_PATH}/Includes)
find_library(PUSOPEN pusopen ${PUSOPEN_PATH}/lib)
target_link_libraries(Svc_TlmChan ${PUSOPEN})


### UTs ###
set(UT_SOURCE_FILES
  "${FPRIME_FRAMEWORK_PATH}/Svc/TlmChan/TlmChanComponentAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/TlmChanTester.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/TlmChanImplTester.cpp"
  "${MDB_PATH}/pusopen_onboard_mdb.c"                    # PUSOpen mission database
)
register_fprime_ut()

