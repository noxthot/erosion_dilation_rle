/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: pict.cc 16 2012-09-05 13:08:00Z fesc $
 *
 * @brief  Implementation for basis methods for ipl::PictImage
 *
 ********************************************************************/

#include "ipl/pict.hh"

#include <algorithm>
#include <functional>
#if defined IPL_USE_OLD_CV
#    include <opencv/cv.h>
#else
#    include <opencv2/core/core.hpp>
#endif

#include "ipl/iplerr.hh"
#include "ipl/log.hh"
#include "ipl/validable.hh"
#include "ipl/range.hh"

using namespace std;

IPL_NS_BEGIN

//! Class logger
template<typename T>
IPL_DEFINE_CLASS_LOGGER(PictImg<T>, ipl.pictimage);

template<typename T>
PictImg<T>::PictImg(N16 sizeX,
                    N16 sizeY)
    : size_(sizeX, sizeY),
      capacity_(sizeX*sizeY),
      data_(0),
      myOwnership_(true),
      fullRoi_(WinP(0,0, sizeX-1, sizeY-1)),
      roi_(fullRoi_)
{
    if (sizeX <= 0)
        throw ParameterError(1, IPL_FNC_NAME);
    if (sizeY <= 0)
        throw ParameterError(2, IPL_FNC_NAME);
    data_ = new PixelType[capacity_];
    IPL_ASSERT_VALID(*this);
}

template<typename T>
PictImg<T>::PictImg(PointN16 const & sz)
    : size_(sz),
      capacity_(sz.x_*sz.y_),
      data_(0),
      myOwnership_(true),
      fullRoi_(WinP(0, 0, sz.x_-1, sz.y_-1)),
      roi_(fullRoi_)
{
    if (size_.x_ <= 0 || size_.y_ <= 0)
        throw ParameterError(1, IPL_FNC_NAME);
    data_ = new PixelType[capacity_];
    IPL_ASSERT_VALID(*this);
}

template<typename T>
PictImg<T>::PictImg(N16 sizeX, N16 sizeY, PixelType * pData,
                    boost::shared_ptr<CamModel const> pCamModel /*=0*/)
    : size_(sizeX, sizeY),
      capacity_(sizeX*sizeY),
      data_(pData),
      myOwnership_(false),
      fullRoi_(WinP(0,0, sizeX-1, sizeY-1)),
      roi_(fullRoi_),
      camModel_(pCamModel)
{
    if (size_.x_ <= 0 || size_.y_ <= 0)
        throw ParameterError(1, IPL_FNC_NAME);
    IPL_ASSERT_VALID(*this);
}

template<typename T>
PictImg<T>::PictImg(PictImg<T> const & other)
    : Validable(other),
      size_(other.size()),
      capacity_(other.capacity_),
      data_(0),
      myOwnership_(true),
      fullRoi_(other.fullRoi_),
      roi_(other.roi_)
{
    IPL_ASSERT_VALID(other);
    data_ = new PixelType[capacity_];
    copy(other.begin(), other.end(), this->begin());
    IPL_ASSERT_VALID(*this);
}

template<typename T>
PictImg<T>::~PictImg()
{
    IPL_ASSERT_VALID(*this);
    if (myOwnership_)
        delete [] data_;
}

template<typename T>
PictImg<T> &
PictImg<T>::operator=(PictImg<T> const & rhs)
{
    IPL_ASSERT_VALID(rhs);
    Validable::operator=(rhs);
    if (this != &rhs) {
        PixelType * p = new PixelType[rhs.capacity_];
        delete [] data_;
        fullRoi_ = rhs.fullRoi_;
        roi_ = rhs.roi_;
        data_ = p;
        myOwnership_ = true;
        capacity_ = rhs.capacity_;
        size_ = rhs.size_;
        copy(rhs.begin(), rhs.end(), this->begin());
        camModel_.reset();
        IPL_ASSERT_VALID(*this);
    }
    return *this;
}

template<typename T>
typename PictImg<T>::RowRange const
PictImg<T>::rowRange(PointN16 const & p, N32 len /*=-1*/)
{
    IPL_ASSERT_VALID(*this);
    N32 x = p.x_, y = p.y_;
    len = adaptLen(p, len, false);
    IPL_ASSERT(len >= 0);
    if (len == 0)
        x = y = 0;
    return make_pair(Iterator(dataPtrAt(x, y)),
                     Iterator(dataPtrAt(x+len, y)));
}

template<typename T>
typename PictImg<T>::ConstRowRange const
PictImg<T>::rowRange(PointN16 const & p, N32 len /*=-1*/) const
{
    IPL_ASSERT_VALID(*this);
    N32 x = p.x_, y = p.y_;
    len = adaptLen(p, len, false);
    IPL_ASSERT(len >= 0);
    if (len == 0)
        x = y = 0;
    return make_pair(ConstIterator(dataPtrAt(x, y)),
                     ConstIterator(dataPtrAt(x+len, y)));
}

template<typename T>
typename PictImg<T>::ColRange const
PictImg<T>::colRange(PointN16 const & p, N32 len /*=-1*/)
{
    IPL_ASSERT_VALID(*this);
    N32 x = p.x_, y = p.y_;
    len = adaptLen(p, len, true);
    IPL_ASSERT(len >= 0);
    if (len == 0)
        x = y = 0;
    return make_pair(ColIterator(dataPtrAt(x, y), this->width()),
                     ColIterator(dataPtrAt(x, y+len), this->width()));
}

template<typename T>
typename PictImg<T>::ConstColRange const
PictImg<T>::colRange(PointN16 const & p, N32 len /*=-1*/) const
{
    IPL_ASSERT_VALID(*this);
    N32 x = p.x_, y = p.y_;
    len = adaptLen(p, len, true);
    IPL_ASSERT(len >= 0);
    if (len == 0)
        x = y = 0;
    return make_pair(ConstColIterator(dataPtrAt(x, y), this->width()),
                     ConstColIterator(dataPtrAt(x, y+len), this->width()));
}

IPL_ANON_NS_BEGIN
template<typename T>
struct LookUp : public unary_function<T,T>
{
    LookUp(vector<T> const & lut)
        : lut_(lut)
    {}
    T operator()(T g) const {
        return lut_[g];
    }
    vector<T> const & lut_;
};
IPL_ANON_NS_END

template<typename T>
PictImg<T> &
PictImg<T>::applyLut(std::vector<PictImg<T>::PixelType> const &)
{
    throw NotImplementedYetError(string("applyLut")+NameType<PixelType>::toString());
}
template<>
PictImg<UN8> &
PictImg<UN8>::applyLut(std::vector<PictImg<UN8>::PixelType> const & lut)
{
    this->pixTransform(LookUp<UN8>(lut));
    return *this;
}

template<>
PictImg<UN8> &
PictImg<UN8>::invert()
{
    IPL_ASSERT_VALID(*this);
    this->pixTransform(bind1st(minus<UN8>(), 255));
    IPL_ASSERT_VALID(*this);
    return *this;
}
template<>
PictImg<N16> &
PictImg<N16>::invert()
{
    IPL_ASSERT_VALID(*this);
    this->pixTransform(negate<N16>());
    IPL_ASSERT_VALID(*this);
    return *this;
}

IPL_ANON_NS_BEGIN
//! Funktor für saturierte Addition eines konstanten Wertes @a val
template<typename PixType>
struct PlusSat : public std::unary_function<PixType, PixType> {
    //! ctr
    /*! @a val ist der Wert, der addiert werden soll
     */
    PlusSat(PixType val)
        : val_(val),
          r_(std::numeric_limits<PixType>::min(),
             std::numeric_limits<PixType>::max())
    {}
    //! Operation
    /*! Führt die Addition <em>p + val</em> aus
     */
    PixType operator()(PixType p) const {
        return r_.clip(p+val_);
    }
private:
    //! Wert, der addiert werden soll
    N32 const val_;
    //! Wertebereich des Pixeltypes
    Range<N32> const r_;
};

//! Funktor für saturierte Subtraction eines konstanten Wertes @a val
template<typename PixType>
struct MinSat : public std::unary_function<PixType, PixType> {
    //! ctr
    /*! @a val ist der Wert, der subtrahiert werden soll
     */
    MinSat(PixType val)
        : val_(val),
          r_(std::numeric_limits<PixType>::min(),
             std::numeric_limits<PixType>::max())
    {}
    //! Operation
    /*! Führt die Subtraktion <em>p - val</em> aus
     */
    PixType operator()(PixType p) const {
        return r_.clip(p-val_);
    }
private:
    //! Wert, der subtrahiert werden soll
    N32 const val_;
    //! Wertebereich des Pixeltyps
    Range<N32> const r_;
};

//! Funktor für absDiff mit einem konstanten Wert @a val
template<typename PixType>
struct AbsDiff : public std::unary_function<PixType, PixType> {
    //! ctr
    /*! @a val ist der Wert für <tt>abs(p-val)</tt>, wenn p der Pixelwert ist
     */
    AbsDiff(PixType val)
        : val_(val)
    {}
    //! Führt die Operation aus
    PixType operator()(PixType p) const {
        return mathli::abs(val_ - p);
    }
private:
    //! Wert, gegen den die Differenz gebildet wird
    N32 const val_;
};
IPL_ANON_NS_END

template<typename T>
PictImg<T> &
PictImg<T>::add(PictImg::PixelType val)
{
    IPLLOG_INFO(IPL_FNC_NAME << " for " << *this << " and val " << N32(val));
    IPL_ASSERT_VALID(*this);
    this->pixTransform(PlusSat<typename PictImg::PixelType>(val));
    IPL_ASSERT_VALID(*this);
    return *this;
}

template<typename T>
PictImg<T> &
PictImg<T>::sub(PictImg::PixelType val)
{
    IPLLOG_INFO(IPL_FNC_NAME << " for " << *this << " and val " << N32(val));
    IPL_ASSERT_VALID(*this);
    this->pixTransform(MinSat<typename PictImg::PixelType>(val));
    IPL_ASSERT_VALID(*this);
    return *this;
}

template<typename T>
PictImg<T> &
PictImg<T>::absDiff(PictImg::PixelType val)
{
    IPLLOG_INFO(IPL_FNC_NAME << " for " << *this << " and val " << N32(val));
    IPL_ASSERT_VALID(*this);
    this->pixTransform(AbsDiff<typename PictImg::PixelType>(val));
    IPL_ASSERT_VALID(*this);
    return *this;
}

template<typename T>
PictImg<T> &
PictImg<T>::doPictOp(PictImg const & rhs, N32 op)
{
    IPL_ASSERT_VALID(rhs);
    if (this->empty() or rhs.empty()) {
        return *this;
    }
    WinP const & bbox1 = this->roi().boundingBox();
    PointN16 const & orig1 = bbox1.upperLeft();
    WinP const & bbox2 = rhs.roi().boundingBox();
    PointN16 const & orig2 = bbox2.upperLeft();
    if (not rhs.fullRoi().includes(bbox1.getTranslate(orig2-orig1))) {
        throw ParameterError(1, string(IPL_FNC_NAME)+": roi outside image");
    }
    cv::Mat src1(bbox1.height(), bbox1.width(), cv::DataType<T>::type,
                 this->dataPtrAt(orig1.x_,orig1.y_),
                 sizeof(T)*this->width());
    cv::Mat src2(bbox1.height(), bbox1.width(), cv::DataType<T>::type,
                 const_cast<Iterator>(rhs.dataPtrAt(orig2.x_, orig2.y_)),
                 sizeof(T)*rhs.width());
    switch (op) {
        case 0:
            cv::add(src1, src2, src1);
            break;
        case 1:
            cv::subtract(src1, src2, src1);
            break;
        case 2:
            cv::absdiff(src1, src2, src1);
            break;
        default:
            IPL_ASSERT(!"wrong operation");
            break;
    }
    IPL_ASSERT_VALID(*this)
    return *this;
}

template<typename T>
PictImg<T> &
PictImg<T>::add(PictImg const & rhs)
{
    IPLLOG_INFO(IPL_FNC_NAME << ": " << *this << " and " << rhs);
    return this->doPictOp(rhs, 0);
}
template<typename T>
PictImg<T> &
PictImg<T>::sub(PictImg const & rhs)
{
    IPLLOG_INFO(IPL_FNC_NAME << ": " << *this << " and " << rhs);
    return this->doPictOp(rhs, 1);
}
template<typename T>
PictImg<T> &
PictImg<T>::absDiff(PictImg const & rhs)
{
    IPLLOG_INFO(IPL_FNC_NAME << ": " << *this << " and " << rhs);
    return this->doPictOp(rhs, 2);
}

template<typename  T>
PictImg<T> &
PictImg<T>::setRoi(Region const & roi) {
    IPL_ASSERT_VALID(roi);
    roi_ = roi;
    roi_.clip(fullRoi_);
    IPL_ASSERT_VALID(*this);
    return *this;
}

template<typename T>
PictImg<T> &
PictImg<T>::detachRoi() {
    roi_ = Region(fullRoi_);
    return *this;
}

template<typename T>
typename PictImg<T>::Ptr
PictImg<T>::crop(WinP const & winp, Trafo2D * pTrBack /* = 0*/) const
{
    IPLLOG_INFO(IPL_FNC_NAME << " with rect " << winp);
    IPL_ASSERT_VALID(*this);
    IPL_ASSERT_VALID(winp);
    if (not this->fullRoi().includes(winp)) {
        throw ParameterError(1, "winp outside image");
    }

    Ptr ret(new PictImg(winp.width(), winp.height()));
    N32 const len = winp.width();
    N32 const ys = winp.upperLeft().y_,
              ye = winp.lowerRight().y_ + 1,
              x = winp.upperLeft().x_;
    auto p = ret->begin();
    for (N32 y = ys; y < ye; ++y) {
        p = copy(dataPtrAt(x, y), dataPtrAt(x+len, y), p);
    }
    ret->setRoi(this->roi_.getTranslate(-winp.upperLeft()));

    if(pTrBack)
    {
        Trafo2D trBack;
        trBack.translate(winp.upperLeft());
        (*pTrBack) *= trBack;
    }
    IPL_ASSERT_VALID(*ret);
    return ret;
}

template<typename T>
std::ostream &
PictImg<T>::print(std::ostream & os) const
{
    return os << "pict<" << NameType<PixelType>::toString() << ">: "
              << size_ << "(sz=" << capacity_ << "->" << static_cast<void*>(data_) << ")";
}

template<typename T>
bool
PictImg<T>::validate() const
{
    bool ok = this->size().x_ > 0 && this->size().y_ > 0
        && capacity_ == this->size().x_*this->size().y_ && data_;
    if (this->empty())
        return ok;
    WinP const & bb = roi_.boundingBox();
    return ok
        && roi_.validate()
        && bb.upperLeft().x_ >= 0
        && bb.upperLeft().y_ >= 0
        && bb.lowerRight().x_ < this->size().x_
        && bb.lowerRight().y_ < this->size().y_
        && fullRoi_.upperLeft().x_ == 0
        && fullRoi_.upperLeft().y_ == 0
        && fullRoi_.lowerRight().x_ == size().x_-1
        && fullRoi_.lowerRight().y_ == size().y_-1;
}

//! Debug Ausgabe @relates PictImg
/*! Diese Überladung ist nötig, um 8Bit Pixel richtig handhaben zu können.
 * @note Da @c std::stringstream und wohl auch @c log4cplus::StringBuffer
 * von @c std::ostream erbt, darf nur diese eine Überladung existieren.
 */
template<typename T>
std::ostream & operator<<(std::ostream & b,
                          typename PictImg<T>::PixelType t)
{
    return b << static_cast<N32>(t);
}


/************************* privater Kram **********************/
template<typename T>
N32
PictImg<T>::adaptLen(PointN16 const & p, N32 len, bool vertical) const
{
    if (p.x_ < 0 || p.x_ >= this->width() || p.y_ < 0 || p.y_ >= this->height())
        return 0;
    if (vertical) {
        if (len < 0 || p.y_ + len > this->height())
            return this->height() - p.y_;
    } else {
        if (len < 0 || p.x_ + len > this->width())
            return this->width() - p.x_;
    }
    return len;
}

IPL_NS_END
