/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008
 *              all rights reserved
 *
 * $Id: winp.cc 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Implementation for WinP
 *
 ********************************************************************/

#include "ipl/winp.hh"

#include <string>

#include "ipl/iplerr.hh"

using namespace std;

IPL_NS_BEGIN

WinP::WinP()
    : ul_(0,0),
      lr_(0,0)
{
    IPL_ASSERT_VALID(*this);
}

WinP::WinP(PointN16 const & ul,
           PointN16 const & lr)
    : ul_(ul),
      lr_(lr)
{
    if (ul_.x_ > lr_.x_)
        throw ParameterError(1, IPL_FNC_NAME);
    if (ul_.y_ > lr_.y_)
        throw ParameterError(2, IPL_FNC_NAME);
    IPL_ASSERT_VALID(*this);
}

WinP::WinP(N16 ulX, N16 ulY, N16 lrX, N16 lrY)
    : ul_(ulX, ulY),
      lr_(lrX, lrY)
{
    if (ul_.x_ > lr_.x_)
        throw ParameterError(1, IPL_FNC_NAME);
    if (ul_.y_ > lr_.y_)
        throw ParameterError(3, IPL_FNC_NAME);
    IPL_ASSERT_VALID(*this);
}

WinP &
WinP::translate(PointN16 const & v)
{
    IPL_ASSERT_VALID(*this);
    ul_ += v;
    lr_ += v;
    IPL_ASSERT_VALID(*this);
    return *this;
}

WinP const
WinP::getTranslate(PointN16 const & v) const
{
    WinP r(*this);
    return r.translate(v);
}

bool
WinP::clip(WinP const & other)
{
    IPL_ASSERT_VALID(*this);
    IPL_ASSERT_VALID(other);
    PointN16 ul(max(this->ul_.x_, other.ul_.x_), max(this->ul_.y_, other.ul_.y_));
    PointN16 lr(min(this->lr_.x_, other.lr_.x_), min(this->lr_.y_, other.lr_.y_));
    if (ul.x_ <= lr.x_ and ul.y_ <= lr.y_ ) {
        this->ul_ = ul;
        this->lr_ = lr;
        IPL_ASSERT_VALID(*this);
        return true;
    } else {
        return false;
    }
}

std::ostream &
WinP::print(std::ostream & os) const
{
    return os << "[" << this->ul_ << "-" << this->lr_ << "]";
}

bool
WinP::validate() const
{
    return ul_.x_ <= lr_.x_
        and ul_.y_ <= lr_.y_;
}

IPL_NS_END
