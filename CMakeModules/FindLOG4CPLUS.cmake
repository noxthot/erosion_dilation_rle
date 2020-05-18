# - Find LOG4CPLUS
# Find the native log4cplus includes and library
#
# Variables used by this module, they can change the default behavior
# and need to be set befor calling find_package:
#
# LOG4CPLUS_USE_STATIC_LIBS Can be set to ON to force the static library.
#                           Default is OFF
#
# This module defines
#     LOG4CPLUS_FOUND  If false, do not try to use LOG4CPLUS.
#     LOG4CPLUS_INCLUDE_DIR  where to find log4cplus/logger.h, etc.
#     LOG4CPLUS_DEFINITIONS Preprocessor defines to use with LOG4CPLUS, use
#         add_definitions(${LOG4CPLUS_DEFINITIONS})
#     LOG4CPLUS_LIBRARIES  the libraries needed to use LOG4CPLUS.
#     LOG4CPLUS_VERSION version of the lib

#***************************************************************
#
# @file
# @author fesc
# @date   Mai 15 2008
# @par    Copyright: &copy; Datacon 2008-2012
#              all rights reserved
#
# $Id: FindLOG4CPLUS.cmake 5845 2012-05-22 11:07:30Z fesc $
#
# @brief   cmake-macro to find log4cplus
#
#***************************************************************

find_path(LOG4CPLUS_INCLUDE_DIR log4cplus/logger.h)

if(LOG4CPLUS_USE_STATIC_LIBS)
  find_library(LOG4CPLUS_LIBRARY_DEBUG NAMES log4cplusSD)
  find_library(LOG4CPLUS_LIBRARY_RELEASE
    NAMES log4cplusS       # on windows
          liblog4cplus.a   # on *NX
          )
  add_definitions(-DLOG4CPLUS_STATIC)
  set(LOG4CPLUS_DEFINITIONS "-DLOG4CPLUS_STATIC")
else()
  find_library(LOG4CPLUS_LIBRARY_DEBUG NAMES log4cplusD)
  find_library(LOG4CPLUS_LIBRARY_RELEASE NAMES log4cplus)
  set(LOG4CPLUS_DEFINITIONS "")
endif()

#grep the version
if(LOG4CPLUS_INCLUDE_DIR)
  file(READ ${LOG4CPLUS_INCLUDE_DIR}/log4cplus/version.h _version_content)
  string(REGEX REPLACE
    ".*#define LOG4CPLUS_VERSION LOG4CPLUS_MAKE_VERSION\\(([0-9]+, [0-9]+, [0-9]+).*$"
    "\\1"
    _log4cplus_version ${_version_content}
    )
  string(REGEX REPLACE ", +" "." LOG4CPLUS_VERSION ${_log4cplus_version})
endif()

include(SelectLibraryConfigurations)
select_library_configurations(LOG4CPLUS)

# handle the QUIETLY and REQUIRED arguments and set LOG4CPLUS_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
if(CMAKE_VERSION VERSION_LESS "2.8.3")
  message(FATAL "need cmake >= 2.8.3")
endif()
find_package_handle_standard_args(LOG4CPLUS
  REQUIRED_VARS LOG4CPLUS_INCLUDE_DIR LOG4CPLUS_LIBRARIES
  VERSION_VAR LOG4CPLUS_VERSION
  )

# fehlende lib ergänzen - fehlt Ws2_32.lib gibts unresolved externals
if(WIN32)
  list(APPEND LOG4CPLUS_LIBRARIES Ws2_32.lib)
endif()

#message(${LOG4CPLUS_LIBRARIES})

mark_as_advanced(LOG4CPLUS_INCLUDE_DIR)
