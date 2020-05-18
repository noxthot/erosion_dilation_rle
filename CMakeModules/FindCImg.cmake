# - Find CImg
# Find the CImg header-only library.
# CImg needs X11 and libpthread to display images on *NIX, libgdi32 on Windows
# therefore we detect this libraries and includepaths too
#
# The module defines
#   CIMG_INCLUDE_DIR path where to find CImg.h.
#   CIMG_LIBRARIES a list of libraries to link against if you use CImg
#       to display images. If we cant detect those libraries (CIMG_LIBRARIES-NOT_FOUND),
#       dont try to display and set cimg_display 0 before including CImg.h
#   CIMG_VERSION the version of the library
# It also introduces an option CIMG_DEBUG_LEVEL. Make shure to pass this
# option in your code before including CImg.h like this
#
# #define cimg_verbosity ${CIMG_DEBUG_LEVEL}
# #include <CImg.h>

#***********************************************************
#
# @file
# @author fesc
# @date   Mai 15 2008
# @par    Copyright: &copy; Datacon 2008-2011
#              all rights reserved
#
# $Id: FindCImg.cmake 5807 2012-05-18 06:44:05Z fesc $
#
# @brief  Settings for cimg-library
#
#***********************************************************

find_path(CIMG_INCLUDE_DIR
  NAMES CImg.h
  )
mark_as_advanced(CIMG_INCLUDE_DIR)
if(CImg_FIND_QUIETLY)
  set(_quiet_arg QUIET)
endif()

# extract the version
if(CIMG_INCLUDE_DIR)
  file(READ ${CIMG_INCLUDE_DIR}/CImg.h
    _cimg_Content
    LIMIT 4000
    )
  string(REGEX REPLACE ".*#define cimg_version ([0-9]+).*" "\\1"
    _version "${_cimg_Content}")
  math(EXPR _major "${_version} / 100")
  math(EXPR _minor "${_version} / 10 % 10")
  math(EXPR _patch "${_version} % 10")
  set(CIMG_VERSION "${_major}.${_minor}.${_patch}"
    CACHE INTERNAL "version of cimg-library")
endif()

# option for debug level
set(
  CIMG_DEBUG_LEVEL
  0
  CACHE
  STRING
  "sets the debug level for the cimg-library; possible values are 0(=debug off), 1 - 4"
  )
mark_as_advanced(
  CIMG_DEBUG_LEVEL
  )
set_property(CACHE CIMG_DEBUG_LEVEL
  PROPERTY STRINGS "0" "1" "2" "3" "4"
  )

#link to different libraries on different platforms
set(CIMG_LIBRARIES)
if(CIMG_INCLUDE_DIR)
  if(WIN32)
    list(APPEND CIMG_LIBRARIES gdi32)
    list(REMOVE_DUPLICATES CIMG_INCLUDE_DIR)
  else()
    find_package(X11 ${_quiet_arg})
    find_package(Threads ${_quiet_arg})
    # we only need the libX11, no others from package X11
    if(X11_FOUND)
      list(APPEND CIMG_LIBRARIES ${X11_X11_LIB})
      list(APPEND CIMG_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
      list(APPEND CIMG_INCLUDE_DIR ${X11_INCLUDE_DIR})
    else()
      set(CIMG_LIBRARIES CIMG_LIBRARIES-NOTFOUND)
    endif()
    list(REMOVE_DUPLICATES CIMG_INCLUDE_DIR)
    list(REMOVE_DUPLICATES CIMG_LIBRARIES)
  endif()
endif()

include("${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake")
if(CMAKE_VERSION VERSION_LESS "2.8.3")
  message(FATAL "need cmake >= 2.8.3")
endif()
#version check requires cmake 2.8.3
find_package_handle_standard_args(CImg
  REQUIRED_VARS CIMG_INCLUDE_DIR
  VERSION_VAR CIMG_VERSION
  )
