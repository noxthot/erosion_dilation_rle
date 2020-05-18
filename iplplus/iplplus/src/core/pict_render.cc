/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Feb 03 2009
 * @par    Copyright: &copy; Datacon 2008-2011
 *              all rights reserved
 *
 * $Id: pict_render.cc 9 2012-09-05 12:06:03Z fesc $
 *
 * @brief  Renderfunktionen in einem Bild
 *
 ********************************************************************/

#include "ipl/pict.hh"

#include <algorithm>

#include "ipl/mathli.hh"
#include "ipl/range.hh"
using namespace ipl;

using namespace std;

IPL_NS_BEGIN

template<typename T>
PictImg<T> &
PictImg<T>::fill(PixelType c /*= PixelType(0)*/)
{
    IPLLOG_DEBUG(IPL_FNC_NAME << " for " << *this);
    auto scan = this->rboBegin();
    auto last = this->rboEnd();
    for ( ; scan != last; ++scan)
        std::fill_n(this->begin(scan), scan->len(), c);
    IPL_ASSERT_VALID(*this);
    return *this;
}

IPL_NS_END
