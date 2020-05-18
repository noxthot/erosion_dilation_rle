/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008
 *              all rights reserved
 *
 * $Id: circle.cc 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Implementation for ipl::Circle
 *
 ********************************************************************/

#include "ipl/circle.hh"

#include <sstream>

#include "ipl/iplerr.hh"
#include "ipl/mathli.hh"

IPL_NS_BEGIN

Circle::Circle(PointF64 const & center,
               F64 radius)
    : SShape(center),
      radius_(radius)
{
    if (radius < 0)
        throw ParameterError(2, IPL_FNC_NAME);
    IPL_ASSERT_VALID(*this);
}

Circle::Circle(Circle const & rhs)
    : SShape(rhs),
      radius_(rhs.radius())
{
    IPL_ASSERT_VALID(rhs);
    IPL_ASSERT_VALID(*this);
}

Circle &
Circle::operator=(Circle const & rhs)
{
    IPL_ASSERT_VALID(*this);
    IPL_ASSERT_VALID(rhs);
    if (this != &rhs) {
        SShape::operator=(rhs);
        center_ = rhs.center();
        radius_ = rhs.radius();
    }
    IPL_ASSERT_VALID(*this);
    return *this;
}

Circle::~Circle()
{}

F64
Circle::area() const
{
    return radius_ * radius_ * mathli::Pi;
}

F64
Circle::circumference() const
{
    return 2 * mathli::Pi * radius_;
}

Circle &
Circle::translate(PointF64 const & v)
{
    center_ += v;
    return *this;
}

Circle &
Circle::rotate(Angle const &)
{
    return *this;
}

Circle const
Circle::getTranslate(PointF64 const & v) const
{
    Circle r(*this);
    return r.translate(v);
}

Circle const
Circle::getRotate(Angle const & /*alpha*/) const
{
    return *this;
}

bool
Circle::validate() const
{
    return radius_ >= 0;
}

std::ostream &
Circle::print(std::ostream & os) const
{
    return os << "circle at " << center_ << ", radius: " << radius_;

}

IPL_NS_END
