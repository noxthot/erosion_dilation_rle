# - Find CPPUNIT
# Find the native cppunit includes and library
# This module defines
#    CPPUNIT_FOUND  If false, do not try to use cppunit.
#    CPPUNIT_INCLUDE_DIR  where to find cppunit/Test.h, etc.
#    CPPUNIT_LIBRARIES  the libraries needed to use cppunit.
#    CPPUNIT_VERSION the version as string

#***************************************************************
#
# @file
# @author fesc
# @date   Mai 15 2008
# @par    Copyright: &copy; Datacon 2008-2011
#              all rights reserved
#
# $Id: FindCppUnit.cmake 5807 2012-05-18 06:44:05Z fesc $
#
# @brief   cmake-macro to find cppunit
#
#***************************************************************

find_path(CPPUNIT_INCLUDE_DIR cppunit/Test.h)

find_library(CPPUNIT_LIBRARY_RELEASE NAMES cppunit)
find_library(CPPUNIT_LIBRARY_DEBUG NAMES cppunitd)

include(SelectLibraryConfigurations)
select_library_configurations(CPPUNIT)

#grep the version
if(CPPUNIT_INCLUDE_DIR)
  file(READ "${CPPUNIT_INCLUDE_DIR}/cppunit/Portability.h" _cppunit_contents)
  string(REGEX REPLACE
    ".*#define CPPUNIT_VERSION  .([0-9]+\\.[0-9]+\\.[0.9]+).*$" "\\1"
    CPPUNIT_VERSION "${_cppunit_contents}"
    )
endif()

# handle the QUIETLY and REQUIRED arguments and set CPPUNIT_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
if(CMAKE_VERSION VERSION_LESS "2.8.3")
  message(FATAL "need cmake >= 2.8.3")
endif()
find_package_handle_standard_args(CppUnit
  REQUIRED_VARS CPPUNIT_LIBRARIES CPPUNIT_INCLUDE_DIR
  VERSION_VAR CPPUNIT_VERSION
  )

mark_as_advanced(CPPUNIT_INCLUDE_DIR)
