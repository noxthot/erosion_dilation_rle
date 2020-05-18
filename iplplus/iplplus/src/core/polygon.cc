/*****************************************************************//**
 *
 * @file
 * @author felix schwitzer
 * @date   Apr 16 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: polygon.cc 16 2012-09-05 13:08:00Z fesc $
 *
 * @brief Implementation for class Polygon
 *
 ********************************************************************/

#include "ipl/polygon.hh"

#include <typeinfo>
#include <string>
#include <utility>
#include <algorithm>
#include <vector>
#include <boost/scoped_array.hpp>

#include "ipl/iplerr.hh"
#include "ipl/trafo2d.hh"
#include "ipl/winp.hh"

#define _(A) (A)

using namespace std;

IPL_NS_BEGIN

//! Class Logger
template<typename T>
IPL_DEFINE_CLASS_LOGGER(Polygon<T>, ipl.polygon);

template<typename T>
Polygon<T>::Polygon()
    : SShape(PointF64())
{
    IPL_ASSERT_VALID(*this);
}

template<typename T>
typename Polygon<T>::WrappedIterator const
Polygon<T>::wrappedBegin() {
    if (this->points_.empty())
        throw EmptyRegionError(std::string(IPL_FNC_NAME) + _(": empty region"));
    return WrappedIterator(this->begin(), &this->points_);
}

template<typename T>
typename Polygon<T>::ConstWrappedIterator const
Polygon<T>::wrappedBegin() const {
    if (this->points_.empty())
        throw EmptyRegionError(std::string(IPL_FNC_NAME) + _(": empty region"));
    return ConstWrappedIterator(this->begin(), &this->points_);
}

// ANSI C code from the article
// "Centroid of a Polygon"
// by Gerard Bashein and Paul R. Detmer,
// (gb@locke.hs.washington.edu, pdetmer@u.washington.edu)
// in "Graphics Gems IV", Academic Press, 1994
// http://www1.acm.org/pubs/tog/GraphicsGems/gemsiv/centroid.c
template<typename T>
F64
Polygon<T>::signedArea() const
{
    IPL_ASSERT_VALID(*this);

    IPLLOG_DEBUG(IPL_FNC_NAME << " for obj " << *this);

    if (sigArea_.upToDate()) {
        IPLLOG_DEBUG(IPL_FNC_NAME << "->" << sigArea_.get()
                    << " (cached)");
        return sigArea_.get();
    }

    F64 atmp = 0;

    if (this->size() < 3) {
        atmp = 0;
    } else {
        auto q = this->begin(),
            e = this->end(),
            p = --this->end();
        while (q != e) {
            F64 ai = p->x_ * q->y_ - p->y_ * q->x_;
            atmp += ai;
            p = q++;
        }
        atmp /= 2;
    }
    sigArea_.update(atmp);
    IPLLOG_DEBUG(IPL_FNC_NAME << "->" << atmp);
    return atmp;
}

template<typename T>
F64
Polygon<T>::area() const
{
    return mathli::abs(signedArea());
}

template<typename T>
F64
Polygon<T>::circumference() const
{
    IPL_ASSERT_VALID(*this);
    IPLLOG_DEBUG(IPL_FNC_NAME << " for obj " << *this);
    if (size() < 2) {
        return 0;
    }
    auto i = this->wrappedBegin();
    auto e = i;
    auto j = i++;
    F64 s = 0;
    do {
        s += (*j - *i).norm();
        ++j, ++i;
    } while (j != e);
    return s;
}

// Adapted C Code from
// http://www1.acm.org/pubs/tog/GraphicsGems/gemsiv/centroid.c
template<typename T>
PointF64 const &
Polygon<T>::center() const
{
    IPL_ASSERT_VALID(*this);

    IPLLOG_DEBUG(IPL_FNC_NAME << " for obj " << *this);

    if (gravity_.upToDate()) {
        IPLLOG_DEBUG(IPL_FNC_NAME << "->" << gravity_.get() << " (cached)");
        return gravity_.get();
    }

    if (this->size() < 3) {
        gravity_.update(PointF64(0,0));
        return gravity_.get();
    }

    F64 ai, ar = 0;
    F64 xtmp = 0, ytmp = 0;

    auto const e = this->end();
    auto q = this->begin(),
        p = --this->end();
    while (q != e) {
        ar += ai = p->x_ * q->y_ - p->y_ * q->x_;
        xtmp += ai * (p->x_ + q->x_);
        ytmp += ai * (p->y_ + q->y_);
        p = q++;
    }
    if (mathli::abs(ar) > this->size()*std::numeric_limits<F64>::epsilon())
        gravity_.update(PointF64(xtmp/(3*ar), ytmp/(3*ar)));
    else {
        IPLLOG_DEBUG(IPL_FNC_NAME << ": area is 0");
        gravity_.update(PointF64(0,0));
    }

    IPLLOG_DEBUG(IPL_FNC_NAME << "->" << gravity_.get());
    return gravity_.get();
}

template<typename T>
void
Polygon<T>::push_back(Point<T> const & p) {
    invalidateCache();
    points_.push_back(p);
    IPLLOG_DEBUG(IPL_FNC_NAME << ": add point " << p << " add end");
}

template<typename T>
void
Polygon<T>::push_front(Point<T> const & p) {
    invalidateCache();
    points_.push_front(p);
    IPLLOG_DEBUG(IPL_FNC_NAME << ": add point " << p << " in front");
}

template<typename T>
Polygon<T> &
Polygon<T>::removeDoublePoints()
{
    if (this->size() <= 1) {
        return *this;
    }
    auto i = points_.begin(),
        e = points_.end(), j = i++;
    while (i != e) {
        if (*j == *i) {
            i = points_.erase(i);
        } else {
            j = i++;
        }
    }
    if (points_.size() > 1 and *points_.begin() == *points_.rbegin()) {
        points_.pop_back();
    }
    return *this;
}

template<typename T>
Polygon<T> &
Polygon<T>::translate(PointF64 const & v)
{
    IPL_ASSERT_VALID(*this);
    IPLLOG_INFO(IPL_FNC_NAME << " by v = " << v);
    Point<T> t = v.as<T>();
    auto p = this->begin();
    auto const e = this->end();
    for ( ; p != e; ++p)
        *p += t;

    if (gravity_.upToDate())
        gravity_.update(gravity_.get() += t);

    IPL_ASSERT_VALID(*this);
    return *this;
}

template<typename T>
Polygon<T> const
Polygon<T>::getTranslate(PointF64 const & v) const
{
    Polygon<T> r(*this);
    return r.translate(v);
}

template<typename T>
Polygon<T> &
Polygon<T>::rotate(Angle const & alpha)
{
    IPL_ASSERT_VALID(*this);
    IPLLOG_INFO(IPL_FNC_NAME << " by " << alpha.deg() << " deg");
    F64 c = alpha.cos(),
        s = alpha.sin();
    F64 cx = this->center().x_,
        cy = this->center().y_;
    auto p = this->begin();
    auto const e = this->end();
    for ( ; p != e; ++p) {
        F64 tx = p->x_ - cx,
            ty = p->y_ - cy;
        F64 x = c*tx - s*ty + cx;
        F64 y = s*tx + c*ty + cy;
        *p = PointF64(x,y).as<T>();
    }
    IPL_ASSERT_VALID(*this);
    return *this;
}

template<typename T>
Polygon<T> const
Polygon<T>::getRotate(Angle const & alpha) const
{
    Polygon<T> r(*this);
    return r.rotate(alpha);
}

template<typename T>
Polygon<T> &
Polygon<T>::transform(Trafo2D const & tr)
{
    invalidateCache();
    auto i = this->begin(),
        e = this->end();
    for ( ; i != e; ++i) {
        *i = tr.apply(*i);
    }
    return *this;
}

template<typename T>
Polygon<T> const
Polygon<T>::getTransform(Trafo2D const & tr) const
{
    Polygon<T> r(*this);
    return r.transform(tr);
}

//! We have no restrictions
template<typename T>
bool
Polygon<T>::validate() const
{
    return true;
}

template<typename T>
std::ostream &
Polygon<T>::print(std::ostream & os) const
{
    return os << "polygon<" << NameType<T>::toString() << "> with "
              << this->size() << " points";
}

template<typename T>
void
Polygon<T>::invalidateCache()
{
    IPLLOG_DEBUG("invalidate cache");
    gravity_.invalidate();
    sigArea_.invalidate();
}

/*****************************************************************************/

//! @name Explicit Instatiations
//@{
template class Polygon<N16>;
template class Polygon<N32>;
template class Polygon<F64>;
//@}

IPL_NS_END
