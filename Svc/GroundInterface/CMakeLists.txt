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
  "${CMAKE_CURRENT_LIST_DIR}/GroundInterfaceComponentAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/GroundInterface.cpp"
  "${MDB_PATH}/pusopen_onboard_mdb.c"                    # PUSOpen mission database
)
set(MOD_DEPS "Utils/Types")
register_fprime_module()

add_definitions(-D_LINUB1804_GCC750)
include_directories(${PUSOPEN_PATH}/Includes)
find_library(PUSOPEN pusopen ${PUSOPEN_PATH}/lib)
target_link_libraries(Svc_GroundInterface ${PUSOPEN})

# Rules based unit testing
set(UT_MOD_DEPS
        STest
        Fw/Types)

set(UT_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/GroundInterfaceComponentAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/Tester.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/TestMain.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/GroundInterfaceRules.cpp"
        "${MDB_PATH}/pusopen_onboard_mdb.c"                     # PUSOpen mission database
        )
# Test Includes for this UT
register_fprime_ut()

