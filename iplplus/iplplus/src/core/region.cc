/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: region.cc 16 2012-09-05 13:08:00Z fesc $
 *
 * @brief  basic methods for ipl::Region
 *
 ********************************************************************/

#include "ipl/region.hh"

#include <limits>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <boost/bind.hpp>
#if defined IPL_USE_OLD_CV
#    include <opencv/cv.h>
#else
#    include <opencv2/core/core.hpp>
#    include <opencv2/imgproc/imgproc.hpp>
#endif

#include "ipl/pict.hh"
#include "ipl/winp.hh"
#include "ipl/iplerr.hh"
#include "ipl/ellipse.hh"
#include "ipl/range.hh"

using namespace std;

IPL_NS_BEGIN

//! Class Logger
IPL_DEFINE_CLASS_LOGGER(Region,ipl.region);

Region::Region()
{
    IPL_ASSERT_VALID(*this);
}

Region::Region(Region const & other)
    : Validable(other),
      rbos_(other.rbos_),
      bbox_(other.bbox_),
      area_(other.area_),
      perimeter_(other.perimeter_),
      center_(other.center_)
{
    IPL_ASSERT_VALID(other);
    IPL_ASSERT_VALID(*this);
}

Region::Region(PictImg<UN8> const & img,
               N32 lo, N32 hi)
{
    createFromPic(img, lo, hi);
}

Region::Region(PictImg<N16> const & img,
               N32 lo, N32 hi)
{
    createFromPic(img, lo, hi);
}

Region::~Region()
{
    IPL_ASSERT_VALID(*this);
}

Region &
Region::operator=(Region const & rhs) {
    Validable::operator=(rhs);
    IPL_ASSERT_VALID(rhs);
    if (this != &rhs) {
        rbos_ = rhs.rbos_;
        bbox_ = rhs.bbox_;
        area_ = rhs.area_;
        perimeter_ = rhs.perimeter_;
        center_ = rhs.center_;
        IPL_ASSERT_VALID(*this);
    }
    return *this;
}

WinP const &
Region::boundingBox() const
{
    IPL_ASSERT_VALID(*this);
    if (this->empty())
        throw EmptyRegionError(string(IPL_FNC_NAME) + ": empty region");
    if (!bbox_.upToDate())
        computeBbox();
    IPL_ASSERT_VALID(*this);
    IPL_ASSERT_VALID(bbox_.get());
    return bbox_.get();
}

IPL_ANON_NS_BEGIN
//! Functor to find the scanline of a Rbo.
struct RowLess : public binary_function<Rbo,N16,bool> {
    //! call-operator
    bool operator()(Rbo const & r, Rbo const & t) const {
        return r.start().y_ < t.start().y_;
    }
};
//! Check if @a x lies in the Rbo @a r.
bool
includesSameY(Rbo const & r, N16 x) {
    return r.start().x_ <= x
        && x < r.start().x_ + r.len();
}
IPL_ANON_NS_END

bool
Region::includes(PointN16 const & pt) const
{
    IPL_ASSERT_VALID(*this);

    IPLLOG_DEBUG(IPL_FNC_NAME << " test point " << pt);
    auto const e = this->end();
    auto r = lower_bound(this->begin(), e, Rbo(PointN16(0,pt.y_),3), RowLess());
    if (r == e || (r->start().y_ != pt.y_)) {
        return false;
    }
    do {
        IPLLOG_DEBUG("check with rbo "  << *r);
        if (includesSameY(*r, pt.x_))
            return true;
    } while (++r != e && r->start().y_ == pt.y_);
    return false;
}

Region &
Region::translate(PointN16 const & v)
{
    IPLLOG_INFO(IPL_FNC_NAME << " " << *this << " with " << v);
    IPL_ASSERT_VALID(*this);
    for (auto i = rbos_.begin();
         i != rbos_.end();
         ++i)
         i->translate(v);
    this->invalidateCaches();
    IPL_ASSERT_VALID(*this);
    return *this;
}

Region const
Region::getTranslate(PointN16 const & v) const
{
    Region r(*this);
    return r.translate(v);
}


Region &
Region::clip(WinP const & win)
{
    IPL_ASSERT_VALID(*this);
    IPL_ASSERT_VALID(win);
    IPLLOG_DEBUG(IPL_FNC_NAME << ": " << *this << " to box " << win);
    if (this->empty()) {
        IPLLOG_DEBUG("empty region: nothing to do");
        return *this;
    }
    WinP const & bb = this->boundingBox();
    N32 const x0 = win.upperLeft().x_,
        y0 = win.upperLeft().y_,
        x1 = win.lowerRight().x_,
        y1 = win.lowerRight().y_;

    if (x0 <= bb.upperLeft().x_
        && y0 <= bb.upperLeft().y_
        && x1 >= bb.lowerRight().x_
        && y1 >= bb.lowerRight().y_) {
        return *this;
    }

    vector<Rbo> const tmp(rbos_);
    rbos_.clear();

    auto r = tmp.begin();
    auto const e = tmp.end();

    try {
        // upper row
        while (r != e && r->start().y_ < y0)
            ++r;
        IPL_ASSERT(r == e || r->start().y_ >= y0);
        while(r != e && r->start().y_ <= y1) {
            IPLLOG_DEBUG("process rbo: " << *r);
            N32 y = r->start().y_;
            N32 xs = r->start().x_;
            N32 xe = xs + r->len() - 1;
            if (xe < x0 || xs > x1)
                ; //nothing to do
            else {
                xs = max(xs, x0);
                xe = min(xe, x1);
                Rbo n(PointN16(xs,y), xe-xs+1);
                this->add(n);
                IPLLOG_DEBUG("new rbo: " << n);
            }
            ++r;
        }
        IPL_ASSERT(r == e || r->start().y_ > y1);
        this->invalidateCaches();
    } catch(...) {
        rbos_ = tmp;
        throw;
    }
    return *this;
}

std::ostream &
Region::print(std::ostream & os) const
{
    os << "region with " << this->nrRbos() << " rbos and bbox ";
    if (bbox_.upToDate()) {
        os << bbox_.get();
    } else {
        os << " not computed yet";
    }
    return os;
}

bool
Region::validate() const
{
    if (this->empty()) {
        return rbos_.empty() and not center_.upToDate();
    }
    bool ok = !bbox_.upToDate() || bbox_.get().validate();
    if (!ok) {
        IPLLOG_ERROR(IPL_FNC_NAME << ": invalid bounding box");
        return ok;
    }
    auto first = this->begin();
    auto last = this->end();
    do {
        ok = ok && first->len() > 0;
        if (bbox_.upToDate()) {
            PointN16 pe(first->start().x_+first->len()-1,
                        first->start().y_);
            ok = bbox_.get().includes(first->start()) && bbox_.get().includes(pe);
        }
    } while (ok && ++first != last);
    if (!ok) {
        IPLLOG_ERROR(IPL_FNC_NAME << ": wrong bounding box");
        return ok;
    }

    auto i2 = this->begin();
    auto i1 = i2++;
    bool od = true;
    while (ok && i2 != last) {
        ok = ok && *i1 < *i2;
        od = od && (i1->start().y_ < i2->start().y_
                    || i1->start().x_+i1->len() < i2->start().x_);
        i1 = i2++;
    }
    if (!ok)
        IPLLOG_ERROR(IPL_FNC_NAME << ": rbos not orderd");
    if (!od)
        IPLLOG_ERROR(IPL_FNC_NAME << ": overlapping rbos");

    return ok && od;
}

Region &
Region::add(Rbo const & s)
{
    IPL_ASSERT(s.len() > 0);
    rbos_.push_back(s);
    return *this;
}

void
Region::computeBbox() const
{
    IPLLOG_INFO(IPL_FNC_NAME << " for " << *this);
    IPL_ASSERT_VALID(*this);
    if (this->nrRbos() == 0) {
        bbox_.invalidate();
        return;
    }
    N16 minx = numeric_limits<N16>::max();
    N16 maxx = numeric_limits<N16>::min();

    auto first = this->begin();
    auto last = this->end();
    // Rbos sind sortiert
    N16 const miny = first->start().y_;
    N16 const maxy = (last-1)->start().y_;

    for ( ; first != last; ++first) {
        N16 sx = first->start().x_;
        N16 ex = sx + first->len() - 1;
        minx = min(minx, sx);
        maxx = max(maxx, ex);
    }
    bbox_.update(WinP(minx, miny, maxx, maxy));
    //IPL_ASSERT_VALID(*this);
    IPLLOG_INFO(IPL_FNC_NAME << " found box " << bbox_.get());
}

void
Region::invalidateCaches()
{
    bbox_.invalidate();
    area_.invalidate();
    perimeter_.invalidate();
    center_.invalidate();
}

template<typename T>
void
Region::createFromPic(PictImg<T> const & img,
                   N32 lo, N32 hi)
{
    IPL_ASSERT_VALID(img);
    IPLLOG_INFO(IPL_FNC_NAME << " from binarization with " << PointN16(lo,hi));
    this->rbos_.clear();
    this->invalidateCaches();
    if (lo > hi) {
        IPLLOG_WARN(IPL_FNC_NAME << " empty binarization range " << PointN16(lo,hi));
        IPL_ASSERT_VALID(*this);
        return;
    }
    auto scan = img.rboBegin();
    auto scanLast = img.rboEnd();

    for ( ; scan != scanLast; ++scan) {
        auto first = img.begin(scan);
        auto last = img.end(scan);
        auto i = first;
        typename PictImg<T>::ConstIterator blobStart;
        do {
            while (i != last &&
                   (*i < lo || *i > hi))
                ++i;
            if (i == last)
                break;
            IPL_ASSERT(i < last);
            blobStart = i;
            while (i != last &&
                   (*i >= lo && *i <= hi))
                ++i;
            this->add(Rbo(PointN16(scan->start().x_+(blobStart-first),
                                   scan->start().y_),
                          i - blobStart));
        } while(i != last);
    }
    IPLLOG_INFO(IPL_FNC_NAME << ": -> " << *this);
    IPL_ASSERT_VALID(*this);
}

/*****************************************************************************/


IPL_NS_END
