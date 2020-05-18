/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Jul 7, 2009
 * @par    Copyright: &copy; Datacon 2008-2011
 *              all rights reserved
 *
 * $Id: trafo2d.cc 16 2012-09-05 13:08:00Z fesc $
 *
 * @brief  Affine 2d Transformationen
 *
 ********************************************************************/

#include "ipl/trafo2d.hh"

#include <limits>
#include <boost/format.hpp>

#include "ipl/angle.hh"
#include "ipl/iplerr.hh"
#include "ipl/mathli.hh"

#define _(A) (A)

IPL_NS_BEGIN

Trafo2D::Trafo2D()
{
    reset();
}

Trafo2D &
Trafo2D::reset()
{
    a00_ = a11_ = 1.0;
    a10_ = a01_ = 0.0;
    vx_ = vy_ = 0.0;
    return *this;
}

Trafo2D &
Trafo2D::rotate(Angle const & alpha)
{
    F64 ca = alpha.cos(),
        sa = alpha.sin();
    Trafo2D old(*this);
    a00_ = ca*old.a00_ - sa*old.a10_;
    a01_ = ca*old.a01_ - sa*old.a11_;
    a10_ = sa*old.a00_ + ca*old.a10_;
    a11_ = sa*old.a01_ + ca*old.a11_;
    vx_ = ca*old.vx_ - sa*old.vy_;
    vy_ = sa*old.vx_ + ca*old.vy_;
    return *this;
}

Trafo2D &
Trafo2D::translate(PointF64 const & v)
{
    vx_ += v.x_;
    vy_ += v.y_;
    return *this;
}

Trafo2D &
Trafo2D::translatePrior(PointF64 const & v)
{
    vx_ += a00_ * v.x_ + a01_ * v.y_;
    vy_ += a10_ * v.x_ + a11_ * v.y_;
    return *this;
}

Trafo2D &
Trafo2D::rotate(Angle const & alpha, PointF64 const & center)
{
    return translate(-center).rotate(alpha).translate(center);
}

Trafo2D &
Trafo2D::scale(F64 s) {
    return scale(s, s);
}

Trafo2D &
Trafo2D::scale(F64 sx, F64 sy)
{
    a00_ *= sx;
    a01_ *= sx;
    vx_ *= sx;
    a10_ *= sy;
    a11_ *= sy;
    vy_ *= sy;
    return *this;
}

Trafo2D &
Trafo2D::scale(F64 s, PointF64 const & center)
{
    return scale(s, s, center);
}

Trafo2D &
Trafo2D::scale(F64 sx, F64 sy, PointF64 const & center)
{
    return translate(-center).scale(sx, sy).translate(center);
}

F64
Trafo2D::det() const
{
    return a00_*a11_ - a01_*a10_;
}

/*! @internal A Rotation has the form [c -s; s c] with c=cos(alpha) and s=sin(alpha)
 */
bool
Trafo2D::galileian() const
{
    F64 const tol = 10*std::numeric_limits<F64>::epsilon();
    return mathli::abs(a00_-a11_) < tol and mathli::abs(det()-1) < tol;
}

Trafo2D const
Trafo2D::inv() const
{
    Trafo2D ret;
    F64 const d = det();
    if (mathli::abs(d) < 10 * std::numeric_limits<F64>::epsilon()) {
        throw ProcessingError(_("transform is singular"));
    }
    ret.a00_ =  a11_ / d;
    ret.a01_ = -a01_ / d;
    ret.a10_ = -a10_ / d;
    ret.a11_ =  a00_ / d;
    ret.vx_ = (a01_*vy_ - a11_*vx_) / d;
    ret.vy_ = (a10_*vx_ - a00_*vy_) / d;
    return ret;
}

Trafo2D const
Trafo2D::operator*(Trafo2D const & rhs) const
{
    Trafo2D ret;
    ret.a00_ = a00_*rhs.a00_ + a01_*rhs.a10_;
    ret.a01_ = a00_*rhs.a01_ + a01_*rhs.a11_;
    ret.a10_ = a10_*rhs.a00_ + a11_*rhs.a10_;
    ret.a11_ = a10_*rhs.a01_ + a11_*rhs.a11_;
    ret.vx_  = a00_*rhs.vx_ + a01_*rhs.vy_ + vx_;
    ret.vy_  = a10_*rhs.vx_ + a11_*rhs.vy_ + vy_;
    return ret;
}

/*! @internal We implement @c op*= with @c op* and not, as usual vice versa,
 * because we can't compute @c op*= inplace
 */
Trafo2D &
Trafo2D::operator*=(Trafo2D const & rhs)
{
    return *this = *this * rhs;
}

PointF64 const
Trafo2D::apply(PointF64 const & pt) const
{
    return PointF64(a00_*pt.x_ + a01_*pt.y_ + vx_,
                    a10_*pt.x_ + a11_*pt.y_ + vy_);
}
PointN32 const
Trafo2D::apply(PointN32 const & pt) const
{
    return PointN32(mathli::roundF(a00_*pt.x_ + a01_*pt.y_ + vx_),
                    mathli::roundF(a10_*pt.x_ + a11_*pt.y_ + vy_));
}
PointN16 const
Trafo2D::apply(PointN16 const & pt) const
{
    return PointN16(mathli::roundF(a00_*pt.x_ + a01_*pt.y_ + vx_),
                    mathli::roundF(a10_*pt.x_ + a11_*pt.y_ + vy_));
}

Trafo2D const
Trafo2D::linearPart() const
{
    Trafo2D ret(*this);
    ret.vx_ = ret.vy_ = 0;
    return ret;
}

/*! @internal Zu lösen ist ein lineares Gleichungssystem für aij, vx, vy.
 * Dieses wurde mit Maple direkt gelöst
 */
Trafo2D const
Trafo2D::fromTriangles(PointF64 const & p,
                       PointF64 const & q,
                       PointF64 const & r,
                       PointF64 const & pNew,
                       PointF64 const & qNew,
                       PointF64 const & rNew)
{
    F64 denom = (r.y_-p.y_)*q.x_ + (p.y_-q.y_)*r.x_ + (q.y_-r.y_)*p.x_;
    if (mathli::abs(denom) < 10 * std::numeric_limits<F64>::epsilon()) {
        throw ProcessingError(_("transform is singular"));
    }
    Trafo2D tmp;
    tmp.a00_ = ((pNew.x_-rNew.x_)*q.y_ + (qNew.x_-pNew.x_)*r.y_
                    + (rNew.x_-qNew.x_)*p.y_) / denom;
    tmp.a01_ = ((rNew.x_-pNew.x_)*q.x_ + (pNew.x_-qNew.x_)*r.x_
                    + (qNew.x_-rNew.x_)*p.x_) / denom;
    tmp.a10_ = ((pNew.y_-rNew.y_)*q.y_ + (qNew.y_-pNew.y_)*r.y_
                    + (rNew.y_-qNew.y_)*p.y_) / denom;
    tmp.a11_ = ((rNew.y_-pNew.y_)*q.x_ + (pNew.y_-qNew.y_)*r.x_
                    + (qNew.y_-rNew.y_)*p.x_) / denom;
    tmp.vx_ = ((r.y_*pNew.x_-rNew.x_*p.y_)*q.x_
                    + (qNew.x_*p.y_-q.y_*pNew.x_)*r.x_
                    + (q.y_*rNew.x_-qNew.x_*r.y_)*p.x_) / denom;
    tmp.vy_ = ((rNew.y_*p.x_-r.x_*pNew.y_)*q.y_
                    + (pNew.y_*q.x_-p.x_*qNew.y_)*r.y_
                    + (r.x_*qNew.y_-rNew.y_*q.x_)*p.y_) / denom;
    if (mathli::abs(tmp.det()) < 10 * std::numeric_limits<F64>::epsilon()) {
        throw ProcessingError(_("transform is singular"));
    }
    return tmp;
}

Trafo2D const
Trafo2D::fromPoses(PointF64 const & from,
                   PointF64 const & to,
                   Angle const & angle /*= Angle()*/)
{
    Trafo2D ret;
    return ret.translate(to-from).rotate(angle, to);
}

std::ostream &
operator<<(std::ostream & os, Trafo2D const & tr)
{
    boost::format fmt("\n|%1$10.5f %2$10.5f %3$10.5f|");
    os << fmt % tr.a00() % tr.a01() % tr.vx();
    os << fmt % tr.a10() % tr.a11() % tr.vy();
    return os;
}

IPL_NS_END
