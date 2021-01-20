####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
# Note: using PROJECT_NAME as EXECUTABLE_NAME
####

set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/GroundInterfaceComponentAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/GroundInterface.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../../Lib/mdb/pusopen_onboard_mdb.c"       # PUSOpen mission database
)
set(MOD_DEPS "Utils/Types")
register_fprime_module()

add_definitions(-D_LINUB1804_GCC750)
include_directories(../../Lib/Includes)
find_library(PUSOPEN pusopen ../../Lib/lib)
# PUSOPEN /home/jonathan/tm/CHESS/05_FS/fprime/Lib/lib/libpusopen.a found
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
        "${CMAKE_CURRENT_LIST_DIR}/../../Lib/mdb/pusopen_onboard_mdb.c"         # PUSOpen mission database
        )
# Test Includes for this UT
register_fprime_ut()

