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
# @brief  Setzt ein kleines Testbeispiel auf
#
#***************************************************************

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

include_directories(
  ${PROJECT_SOURCE_DIR}/iplplus/include
  ${PROJECT_BINARY_DIR}/iplplus/include
  ${CMAKE_CURRENT_BINARY_DIR}
)

if(IPL_BOARD_CIMG)
  # scratch board
  add_executable(ipltest main.cc)
  target_link_libraries(ipltest iplplus ${CIMG_LIBRARIES})
else()
  message(STATUS "iplplus configured w/o CImgBoard, skipping examples")
endif()
