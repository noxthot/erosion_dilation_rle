/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2011
 *              all rights reserved
 *
 * $Id: rect.cc 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Implementation for ipl::Rect
 *
 ********************************************************************/

#include "ipl/rect.hh"

#include "ipl/iplerr.hh"
#include "ipl/winp.hh"
#include "ipl/mathli.hh"
#include "ipl/trafo2d.hh"

using namespace std;

IPL_NS_BEGIN

Rect::Rect(PointF64 const & center,
           PointF64 const & size,
           Angle const & angle /*= 0*/)
    : SShape(center),
      size_(size),
      alpha_(angle)
{
    if (size_.x_ < 0 || size_.y_ < 0)
        throw ParameterError(2, IPL_FNC_NAME);
    IPL_ASSERT_VALID(*this);
}

Rect::Rect(WinP const & win)
    : SShape(PointF64(0,0))
{
    IPL_ASSERT_VALID(win);
    center_ = win.upperLeft() + win.lowerRight();
    center_ *= 0.5;
    size_ = win.lowerRight() - win.upperLeft() + PointN16(1,1);
    alpha_ = Angle();
    IPL_ASSERT_VALID(*this);
}

Rect::Rect(Rect const & rhs)
    : SShape(rhs),
      size_(rhs.size()),
      alpha_(rhs.angle())
{
    IPL_ASSERT_VALID(rhs);
    IPL_ASSERT_VALID(*this);
}

Rect &
Rect::operator=(Rect const & rhs)
{
    IPL_ASSERT_VALID(*this);
    IPL_ASSERT_VALID(rhs);
    SShape::operator=(rhs);
    if (this != &rhs) {
        size_ = rhs.size();
        alpha_ = rhs.angle();
    }
    return *this;
}

Rect::~Rect()
{}

F64
Rect::area() const
{
    return size_.x_ * size_.y_;
}

F64
Rect::circumference() const
{
    return 2 * (size_.x_ + size_.y_);
}

Rect &
Rect::translate(PointF64 const & v)
{
    center_ += v;
    return *this;
}

Rect &
Rect::rotate(Angle const & alpha)
{
    alpha_ += alpha;
    return *this;
}

Rect const
Rect::getTranslate(PointF64 const & v) const
{
    Rect r(*this);
    return r.translate(v);
}

Rect const
Rect::getRotate(Angle const & alpha) const
{
    Rect r(*this);
    return r.rotate(alpha);
}

std::ostream &
Rect::print(std::ostream & os) const
{
    return os << "[" << center_ << " + " << size_ << " (" << alpha_ << ")]";
}

bool
Rect::validate() const
{
    return size_.x_ >= 0 && size_.y_ >= 0;
}

IPL_NS_END
