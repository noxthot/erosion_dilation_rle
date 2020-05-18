/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008
 *              all rights reserved
 *
 * $Id: ellipse.cc 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Implementation for ipl::Ellipse
 *
 ********************************************************************/

#include "ipl/ellipse.hh"

#include <sstream>

#include "ipl/mathli.hh"
#include "ipl/iplerr.hh"

IPL_NS_BEGIN

Ellipse::Ellipse(PointF64 const & center,
                 F64 ra,
                 F64 rb,
                 Angle const & alpha /*=0*/)
    : SShape(center),
      ra_(ra),
      rb_(rb),
      alpha_(alpha)
{
    if (ra < 0)
        throw ParameterError(3, IPL_FNC_NAME);
    if (rb < 0)
        throw ParameterError(4, IPL_FNC_NAME);
    IPL_ASSERT_VALID(*this);
}

Ellipse::Ellipse(Ellipse const & rhs)
    : SShape(rhs),
      ra_(rhs.ra()),
      rb_(rhs.rb()),
      alpha_(rhs.angle())
{
    IPL_ASSERT_VALID(rhs);
    IPL_ASSERT_VALID(*this);
}

Ellipse &
Ellipse::operator=(Ellipse const & rhs)
{
    IPL_ASSERT_VALID(rhs);
    IPL_ASSERT_VALID(*this);
    SShape::operator=(rhs);
    if (this != &rhs) {
        ra_ = rhs.ra();
        rb_ = rhs.rb();
        alpha_ = rhs.angle();
    }
    IPL_ASSERT_VALID(*this);
    return *this;
}

Ellipse::~Ellipse()
{}

F64
Ellipse::area() const
{
    IPL_ASSERT_VALID(*this);
    return ra_ * rb_ * mathli::Pi;
}

F64
Ellipse::circumference() const
{
//    F64 e = mathli::sqrt(1 - rb_/ra_*rb_/ra_);
//    return 4 * ra_ * std::tr1::comp_ellint_2(e);
    return mathli::Pi * (3*(ra_ + rb_) - mathli::sqrt((3*ra_+rb_) * (ra_+3*rb_)));
}

Ellipse &
Ellipse::translate(PointF64 const & v)
{
    center_ += v;
    IPL_ASSERT_VALID(*this);
    return *this;
}

Ellipse &
Ellipse::rotate(Angle const & alpha)
{
    alpha_ += alpha;
    IPL_ASSERT_VALID(*this);
    return *this;
}

Ellipse const
Ellipse::getTranslate(PointF64 const & v) const
{
    Ellipse r(*this);
    return r.translate(v);
}

Ellipse const
Ellipse::getRotate(Angle const & alpha) const
{
    Ellipse r(*this);
    return r.rotate(alpha);
}

bool
Ellipse::validate() const
{
    return ra_ >= 0 && rb_ >= 0;
}

std::ostream &
Ellipse::print(std::ostream & os) const
{
    return os << "ellipse at " << center_ << "axes: " << PointF64(ra_, rb_)
                    << "(" << alpha_ << ")";
}

IPL_NS_END
