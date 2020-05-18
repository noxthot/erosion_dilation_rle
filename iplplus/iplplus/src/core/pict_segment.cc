/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008
 *              all rights reserved
 *
 * $Id: pict_segment.cc 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  segmentation methods for ipl::PictImage
 *
 ********************************************************************/

#include "ipl/pict.hh"
#include "ipl/iplerr.hh"

IPL_NS_BEGIN

template<typename T>
Region const
PictImg<T>::binarize(PixelType lo,
                     PixelType hi) const
{
    return Region(*this, lo, hi);
}


IPL_NS_END
