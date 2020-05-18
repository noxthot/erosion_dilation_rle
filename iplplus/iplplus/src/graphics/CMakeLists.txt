#***************************************************************
#
# \file
# \author dast
# \date   Feb 11 2009
# \par    Copyright: &copy; Datacon 2008
#              all rights reserved
#
# $Id: CMakeLists.txt 1 2012-09-05 10:26:59Z fesc $
#
# \brief  list of sources for cmake
#
#***************************************************************

set(SOURCES board.cc)

if(IPL_BOARD_CIMG)
  list(APPEND SOURCES cimgboard.cc)
endif()
