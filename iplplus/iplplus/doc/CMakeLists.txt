#**************************************************************
#
# @file
# @author fesc
# @date   Apr 14 2008
# @par    Copyright: &copy; Datacon 2008-2012
#              all rights reserved
#
# $Id: CMakeLists.txt 1 2012-09-05 10:26:59Z fesc $
#
# @brief generate doxygen documentation
#
#**************************************************************

if(DOXYGEN_DOT_EXECUTABLE)
  set(HAVE_DOT YES)
else()
  set(HAVE_DOT NO)
endif()
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
)

add_custom_target(
  api_doc
  COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
)

install(
  DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/iplplus_api_doc/
  DESTINATION ${CMAKE_INSTALL_DOCDIR}
  COMPONENT doc
)
