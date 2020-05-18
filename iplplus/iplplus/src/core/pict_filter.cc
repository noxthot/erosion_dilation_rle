/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Jan 27 2009
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: pict_filter.cc 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Filterung von Bildern
 *
 ********************************************************************/

#include "ipl/pict.hh"

#include <type_traits>
#if defined IPL_USE_OLD_CV
#    include <opencv/cv.h>
#else
#    include <opencv2/core/core.hpp>
#    include <opencv2/imgproc/imgproc.hpp>
#endif


using namespace std;

IPL_NS_BEGIN


template<typename T>
typename PictImg<T>::Ptr
PictImg<T>::morph(MorphType type, N32 iterations, MorphElement element, PointN16 const & ksz) const
{
    // OpenCV open/close with even sized kernels is not symetric
    // -> do not allow even sized kernels
    if (not (ksz.x_%2!=0 and ksz.y_%2!=0)) {
        throw ParameterError(1, "PictImg::morph needs odd kernel size");
    }
    // ich wrappe brutal zu OpenCV, daher hier einmal überprüfen
    IPL_STATIC_ASSERT(N32(MorphTypeErode)==N32(cv::MORPH_ERODE),
                      "invalid type");
    IPL_STATIC_ASSERT(N32(MorphTypeDilate)==N32(cv::MORPH_DILATE),
                      "invalid type");
    IPL_STATIC_ASSERT(N32(MorphTypeOpen)==N32(cv::MORPH_OPEN),
                      "invalid type");
    IPL_STATIC_ASSERT(N32(MorphTypeClose)==N32(cv::MORPH_CLOSE),
                      "invalid type");
    IPL_STATIC_ASSERT(N32(MorphTypeGradient)==N32(cv::MORPH_GRADIENT),
                      "invalid type");
    IPL_STATIC_ASSERT(N32(MorphTypeTophat)==N32(cv::MORPH_TOPHAT),
                      "invalid type");
    IPL_STATIC_ASSERT(N32(MorphTypeBlackhat)==N32(cv::MORPH_BLACKHAT),
                      "invalid type");

    IPL_STATIC_ASSERT(N32(MorphElementRectangle)==N32(cv::MORPH_RECT),
                      "invalid element");
    IPL_STATIC_ASSERT(N32(MorphElementCross)==N32(cv::MORPH_CROSS),
                      "invalid element");
    IPL_STATIC_ASSERT(N32(MorphElementEllipse)==N32(cv::MORPH_ELLIPSE),
                      "invalid element");


    IPLLOG_INFO(IPL_FNC_NAME << " for pic " << *this << " with kernelsize " << ksz);
    IPL_ASSERT_VALID(*this);
    PointN16 const sz(PointN16(this->roi().boundingBox().width(),
                               this->roi().boundingBox().height()));
    PointN16 const & orig(this->roi().boundingBox().upperLeft());
    Ptr ret(new PictImg(sz));
    try {
        cv::Mat const src(sz.y_, sz.x_, cv::DataType<PixelType>::type,
                          data_ + orig.x_ + orig.y_*size_.x_,
                          sizeof(PixelType)*size_.x_);
        cv::Mat dst(sz.y_, sz.x_, cv::DataType<PixelType>::type, ret->data_);
        // OpenCV is buggy for iterations > 1 with rectangle element
        // workaround here
        if ((iterations > 1) && (element == MorphElementRectangle))
        {
            //make the kernel bigger and use 1 iteration
            cv::Mat kernel = cv::getStructuringElement(element, cv::Size(1 + iterations*(ksz.x_-1),
                                                                         1 + iterations*(ksz.y_-1)));
            cv::morphologyEx(src, dst, type, kernel, cv::Point(-1,-1), 1);
        }
        else
        {
            cv::Mat kernel = cv::getStructuringElement(element, cv::Size(ksz.x_,ksz.y_));
            cv::morphologyEx(src, dst, type, kernel, cv::Point(-1,-1), iterations);
        }
    } catch(cv::Exception & e) {
        throw ProcessingError(e.err);
    }
    ret->roi_ = this->roi().getTranslate(-orig);
    IPL_ASSERT_VALID(*ret);
    return ret;
}

IPL_NS_END
