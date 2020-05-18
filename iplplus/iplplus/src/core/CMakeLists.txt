#***************************************************************
#
# @file
# @author fesc
# @date   Apr 14 2008
# @par    Copyright: &copy; Datacon 2008-2011
#              all rights reserved
#
# $Id: CMakeLists.txt 12 2012-09-05 12:17:22Z fesc $
#
# @brief  list of sources for cmake
#
#***************************************************************

set(SOURCES circle.cc ellipse.cc iplerr.cc ipltypes.cc iplerr.cc
            #sammelt alle pict-Implementierungen und instanziiert explizit
            #alle anderen pict_xxx.cc Sourcefiles dürfen hier nicht auftauchen,
            #sondern müssen in pict_instantiate.cc includiert werden
            pict_instantiate.cc
            polygon.cc rbo.cc rect.cc
            point.cc
            region.cc region_create.cc region_set.cc region_morph.cc winp.cc
            trafo2d.cc)
