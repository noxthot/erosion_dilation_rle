/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2011
 *              all rights reserved
 *
 * $Id: pict_serialize.cc 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  file IO for ipl::PictImage
 *
 ********************************************************************/

#include "ipl/pict.hh"

#include <string>
#include <fstream>

#include "ipl/iplerr.hh"

//! special hack, da cimg nur hier verwendet wird
//@{
#define cimg_display  0
#define cimg_debug 0
//@}
#if defined(IPL_HAVE_PNG)
#   define cimg_use_png
#endif
#if defined(IPL_HAVE_JPEG)
#   define cimg_use_jpeg
#endif
#if defined(IPL_HAVE_TIFF)
#   define cimg_use_tiff
#endif

#include <CImg.h>

IPL_NS_BEGIN

template<typename T>
PictImg<T>::PictImg(std::string const & fn) try
    : size_(0, 0),
      capacity_(0),
      data_(0),
      myOwnership_(true),
      roi_(WinP(0, 0, 0, 0))
{
    IPLLOG_INFO("read picture from " << fn);
#if defined IPL_HAVE_TIFF
    //shut up warnings about IHDTIFF-tags
    TIFFSetWarningHandler(0);
#endif
    cimg_library::CImg<T> tmp(fn.c_str());
    if (tmp.depth() == 1 and tmp.spectrum() == 3) {
        PictImg<T> t2(tmp.width(), tmp.height());
        IPLLOG_INFO(fn + " is a RGB image, convert to gray");
        auto i = t2.begin(), e = t2.end();
        auto r = tmp.data(0,0,0,0),
             g = tmp.data(0,0,0,1),
             b = tmp.data(0,0,0,2);
        for (; i != e; ++i, ++r, ++g, ++b) {
            *i = mathli::roundF(0.299 * *r + 0.587 * *g + 0.114 * *b);
        }
        operator=(t2);
    } else {
        if (tmp.depth() > 1 or tmp.spectrum() > 1) {
            IPLLOG_WARN(fn + " is a multidimensional image, using first component");
        }
        PictImg<T> t2(tmp.width(), tmp.height(), tmp.begin());
        operator=(t2);
    }
    IPL_ASSERT_VALID(*this);
} catch (cimg_library::CImgIOException & e) {
    throw IoError(e.what());
}

template<typename T>
void
PictImg<T>::save(std::string const & fn) const
try {
    PictImg<T> tmp(*this);
    IPLLOG_INFO("save picture to " << fn);
    tmp.setRoi(tmp.roi().complement(&tmp.fullRoi()));
    tmp.fill(0);
    cimg_library::CImg<T> ci(&tmp.data_[0], size().x_, size().y_);
    ci.save(fn.c_str());
} catch (cimg_library::CImgIOException & e) {
    throw IoError(e.what());
}

IPL_NS_END
