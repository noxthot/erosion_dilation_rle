#****************************************************************//**
#
# @file
# @author fesc
# @date   Mai 14 2009
# @par    Copyright: &copy; Datacon 2008-2012
#              all rights reserved
#
# $Id: CSS-Macros.cmake 5807 2012-05-18 06:44:05Z fesc $
#
# @brief Various macros for the css project
#
#*******************************************************************/

include(CMakeParseArguments)

# svn_revision(<dir> <prefix>)
#
# Determines  the subversion revision  number of  a svn  working copy  <dir> and
# stores  it  under  <prefix>_SVN_REVISION.  If  <dir>  is not  a  wc  then  use
# 'exported' like the svnversion tool does.
#
macro(svn_revision dir prefix)
  find_program(SVNVERSION_EXECUTABLE svnversion)
  mark_as_advanced(SVNVERSION_EXECUTABLE)
  if(SVNVERSION_EXECUTABLE)
    # the subversion commands should be executed with the C locale, otherwise
    # the message (which are parsed) may be translated, Alex
    SET(_SVN_REVISION_SAVED_LC_ALL "$ENV{LC_ALL}")
    SET(ENV{LC_ALL} C)
    execute_process(
      COMMAND ${SVNVERSION_EXECUTABLE} -n
      WORKING_DIRECTORY ${dir}
      OUTPUT_VARIABLE ${prefix}_SVN_REVISION
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    # restore the previous LC_ALL
    SET(ENV{LC_ALL} ${_SVN_REVISION_SAVED_LC_ALL})
    if(${prefix}_SVN_REVISION MATCHES ":")
      message(FATAL_ERROR
        "workdir (rev=${${prefix}_SVN_REVISION}}) not up to date, run 'svn update' first"
        )
    endif()
  else()
    message(STATUS "svnversion not found")
  endif()
endmacro()

# css_compose_version_string(<prefix> [<with_svn_rev>])
#
# Generate a version string
#     <prefix>_VERSION
# in  the form  "MM.mm.pp".  If <with_svn_rev>  is  TRUE, then  append also  the
# SVN_REVISION. This macro needs that the versions
#     <prefix>_VERSION_MAJOR ... <prefix>_SVN_REVISION
# are set before calling the macro
#
macro(css_compose_version_string prefix)
  set(_with_svn_rev "${ARGV1}")
  set(${prefix}_VERSION
    "${${prefix}_VERSION_MAJOR}.${${prefix}_VERSION_MINOR}.${${prefix}_VERSION_PATCH}"
    )
  if(_with_svn_rev)
    set(${prefix}_VERSION "${${prefix}_VERSION}-${${prefix}_SVN_REVISION}")
  endif()
endmacro()

# css_output_dir(prefix [dirname])
#
# Sets the output directory for all binaries in a project <prefix> to
#     <prefix>_BINARY_DIR/<dirname>
# The <dirname> defaults to 'bin'. The macro also creates a cache entry
#     <prefix>_OUTPUT_DIRECTORY
#
macro(css_output_dir prefix)
  if(NOT ${prefix}_OUTPUT_DIRECTORY)
    if(${ARGC} EQUAL 1)
      set(_bindir "bin")
    else()
      set(_bindir ${ARGV1})
    endif()
    set(${prefix}_OUTPUT_DIRECTORY
      ${PROJECT_BINARY_DIR}/${_bindir}
      CACHE PATH
      "Single output directory for building all binaries"
      )
    mark_as_advanced(${prefix}_OUTPUT_DIRECTORY)
  endif()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${${prefix}_OUTPUT_DIRECTORY})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${${prefix}_OUTPUT_DIRECTORY})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${${prefix}_OUTPUT_DIRECTORY})
endmacro()

# css_set_buildtype(<buildtype>)
#
# Sets the  variable CSS_BUILD_TYPE as  a option in  the cmake-gui and  fills it
# with <buildtype>.  On makefile generators  this variable overrides  the global
# CMAKE_BUILD_TYPE.
#
function(css_set_buildtype buildtype)
  set(CSS_BUILD_TYPE ${buildtype} CACHE STRING "build type, one of Debug Release RelWithDebInfo MinSizeRel")
  set_property(CACHE CSS_BUILD_TYPE
    PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel"
    )
  if (CMAKE_GENERATOR MATCHES Makefile)
    set(CMAKE_BUILD_TYPE ${CSS_BUILD_TYPE}
      CACHE STRING "forced to CSS_BUILD_TYPE"
      FORCE
      )
  endif()
endfunction()

## css_set_experimental_option([initial value])
# stellt die Option CSS_EXPERIMENTAL_BUILD als Cacheeintrag zur Verfügung
#
function(css_set_experimental_option)
  set(_initial_val ${ARG0})
  if(${ARGC} EQUAL 0)
    set(_initial_val OFF)
  endif()
  option(CSS_EXPERIMENTAL_BUILD
    "set up the project for an experimental (ci,nightly) build"
    ${_initial_val}
    )
endfunction()

# css_deb_control_extra(<outvar>
#        [POSTRM]
#        [POSTINST]
#        [CONFFILES files]
#   )
#
# Write a standard postrm and postinst script for a shared library if POSTRM and
# POSTINST  are set  respectively  into CMAKE_CURRENT_BINARY_DIR.  Write also  a
# conffile with the list of files if CONFFILES is passed.
#
# Normally you  use this function  to write the  files and fill in  the variable
# CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA passed as <outvar>.
#
function(css_deb_control_extra outvar)
  set(options POSTRM POSTINST)
  set(multiArgs CONFFILES)
  cmake_parse_arguments(GENCONTROL "${options}" "" "${multiArgs}" ${ARGN})
  if(GENCONTROL_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "invalid call of css_deb_control_extra")
  endif()
  set(_out)
  if(GENCONTROL_POSTRM)
    set(_fn ${CMAKE_CURRENT_BINARY_DIR}/postrm)
    file(WRITE ${_fn}
"#!/bin/sh
set -e
if [ \"$1\" = \"remove\" ]; then
    ldconfig
fi
"
      )
    list(APPEND _out "${_fn}")
  endif()
  if(GENCONTROL_POSTINST)
    set(_fn ${CMAKE_CURRENT_BINARY_DIR}/postinst)
    file(WRITE ${_fn}
"#!/bin/sh
set -e
if [ \"$1\" = \"configure\" ]; then
    ldconfig
fi
"
      )
    list(APPEND _out "${_fn}")
  endif()
  if(GENCONTROL_CONFFILES)
    set(_fn ${CMAKE_CURRENT_BINARY_DIR}/conffiles)
    file(WRITE ${_fn} "")
    foreach(_f IN LISTS GENCONTROL_CONFFILES)
      file(APPEND ${_fn} "${_f}\n")
    endforeach()
    list(APPEND _out ${_fn})
  endif()
  # thats the cmake way for a returnvalue in a function
  set(${outvar} "${_out}" PARENT_SCOPE)
endfunction()

# css_swigruby_installdir(<outvar>)
#
# Find  a suitable  installation path  for a  swig generated  ruby  module. This
# function uses the rubyinterpreter and rbconfig to determine this path.
#
function(css_swigruby_installdir outvar)
  find_package(Ruby)
  if(NOT RUBY_FOUND OR NOT RUBY_SITEARCH_DIR)
    message(SEND_ERROR "can't determine ruby-module installdir")
  endif()
  set(${outvar} ${RUBY_SITEARCH_DIR} PARENT_SCOPE)
endfunction()

# css_swigpython_installdir(<outvar>)
#
# Find  a suitable  installation path  for a  swig generated  ruby  module. This
# function uses  the python  interpreter and the  sysconfig module  to determine
# this path.
#
function(css_swigpython_installdir outvar)
  find_package(PythonInterp)
  if(NOT PYTHONINTERP_FOUND OR NOT PYTHON_EXECUTABLE)
    message(SEND_ERROR "can't determine python-module installdir")
  endif()
  execute_process(COMMAND
    ${PYTHON_EXECUTABLE} -c "from distutils import sysconfig; print sysconfig.get_python_lib(1,0)"
    OUTPUT_VARIABLE _out
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _res
    ERROR_VARIABLE _err
    )
  if(_res)
    message(SEND_ERROR "can't determine python-install-dir: ${_err}")
  endif()
  set(${outvar} ${_out} PARENT_SCOPE)
endfunction()
