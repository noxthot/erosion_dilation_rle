#***************************************************************
#
# @file
# @author fesc
# @date   Apr 14 2008
# @par    Copyright: &copy; Datacon 2008-2012
#              all rights reserved
#
# $Id: CMakeLists.txt 1 2012-09-05 10:26:59Z fesc $
#
# @brief  Setup für die Unittests
#
# Alle Unittests werden hier gesammelt und registriert, indem die Liste
# TESTS_TO_RUN erweitert wird.
#
#***************************************************************

include_directories(${CPPUNIT_INCLUDE_DIR})
include_directories(
  ${PROJECT_SOURCE_DIR}/iplplus/include
  ${PROJECT_BINARY_DIR}/iplplus/include
  ${CMAKE_CURRENT_BINARY_DIR}
  )
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.hh.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.hh
  )

# Liste mit den Tests, diese einfach erweitern
set(TESTS_TO_RUN
  test_region_morph
  )

# initialisiere das Logsystem
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN
"
#if defined(IPL_HAVE_LOG4CPLUS)
    log4cplus::BasicConfigurator::doConfigure();
    ipl::iplLogger.setLogLevel(log4cplus::FATAL_LOG_LEVEL);
#endif
// shut up the compiler
localTesting();
"
)
create_test_sourcelist(DUMMY_NAME
  test_driver.cc
  ${TESTS_TO_RUN}
  EXTRA_INCLUDE "./config.hh"
  )

add_executable(test_driver test_driver.cc ${TESTS_TO_RUN})
target_link_libraries(test_driver iplplus ${CPPUNIT_LIBRARIES})

foreach(test IN LISTS TESTS_TO_RUN)
  get_filename_component(TNAME ${test} NAME_WE)
  add_test(${test} ${IPL_OUTPUT_DIRECTORY}/test_driver ${TNAME})
endforeach()

### install all tests
# See http://www.cmake.org/Wiki/CTest:TestWithoutBuild
install(TARGETS test_driver
  RUNTIME DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/ipl
  COMPONENT testing)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/adapt-testpaths.rb
  ${CMAKE_CURRENT_SOURCE_DIR}/run-installed-test.sh
  ${CMAKE_BINARY_DIR}/DartConfiguration.tcl
  ${CMAKE_CURRENT_BINARY_DIR}/CTestTestfile.cmake
  ${CMAKE_BINARY_DIR}/log.cfg
  DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/ipl
  COMPONENT testing)
install(DIRECTORY
  ${CMAKE_SOURCE_DIR}/images
  DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/ipl
  COMPONENT testing
  PATTERN ".svn" EXCLUDE)
install(DIRECTORY
  ${CMAKE_SOURCE_DIR}/tests/testdata
  DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/ipl
  COMPONENT testing
  PATTERN ".svn" EXCLUDE)
