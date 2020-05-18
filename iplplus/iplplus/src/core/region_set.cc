/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Mai 29 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: region_set.cc 16 2012-09-05 13:08:00Z fesc $
 *
 * @brief  Set Operations of Regions
 *
 ********************************************************************/

#include "ipl/region.hh"

#include <algorithm>
#include <functional>
#include <vector>
#include <iterator>

#include "ipl/iplerr.hh"

using namespace std;

IPL_NS_BEGIN

IPL_ANON_NS_BEGIN

//! Union of two Rbos.
/*! Compute the union of a Rbo @a r and an intervall with end @a xe. We don't
 * need the start point of the intervall but we assume, that it is less then
 * the start point of @a r.
 */
inline bool
merge(N32 & xe, Rbo const & r)
{
    N32 ts = r.start().x_,
        te = ts + r.len();
    if (ts > xe)
        return false;
    if (te > xe)
        xe = te;
    return true;
}

//! Compare Functor
struct IterSorter : public binary_function<Region::RboIterator,
                                           Region::RboIterator,
                                           bool>
{
    //! Compare Operation
    bool operator()(Region::RboIterator i1,
                    Region::RboIterator i2) const {
        return *i1 < *i2;
    }
};
IPL_ANON_NS_END

/*!
 * @todo Optimize, avoid the temporary vectors.
 *
 * @internal Iterate simultaneously through the Rbo's of the first Region (@a r)
 * and the second Region(@a s). Because the Rbo's are sorted row-wise, we can:
 * - Add all Rbo's from the upper part, that belongs only to one of the two regions.
 * - If in a certain row we have only rbos from @a r or @a s, add those Rbo's.
 * - Find all Rbos of a row @a iters1 for the first Region and @a iters2 for the
 *   second, store both sets sorted in a vector @a itersMerged. Build the compactification
 *   of these Rbos (merge if there are overlapping Rbo's) and add these Rbos to
 *   the result.
 * - Process the remaining lower part as the upper part.
 */
Region const
Region::unions(Region const & other) const
{
    IPLLOG_INFO(IPL_FNC_NAME << ": " << *this << " and " << other);
    IPL_ASSERT_VALID(*this);
    IPL_ASSERT_VALID(other);

    if (this->empty())
        return other;
    if (other.empty())
        return *this;

    vector<RboIterator> iters1, iters2, itersMerged;

    Region reg;
    auto r = this->begin(),
        s = other.begin(),
        rend = this->end(),
        send = other.end();

    if (*s < *r) {
        swap(r,s), swap(rend,send);
    }
    IPL_ASSERT(r != rend && s != send);
    N32 const y = s->start().y_;
    while (r != rend && r->start().y_ < y) { // upper part
        reg.add(*r);
        ++r;
    }
    IPL_ASSERT(r == rend || r->start().y_ >= s->start().y_);
    while (r != rend && s != send) {
        if (r->start().y_ < s->start().y_) {
            reg.add(*r);
            ++r;
        } else if (s->start().y_ < r->start().y_) {
            reg.add(*s);
            ++s;
        } else {
            iters1.clear();
            iters2.clear();
            itersMerged.clear();
            N32 const y = r->start().y_;
            IPL_ASSERT(s->start().y_ == y);
            do {
                iters1.push_back(r);
            } while (++r != rend && r->start().y_ == y);
            do {
                iters2.push_back(s);
            } while (++s != send && s->start().y_ == y);
            IPL_ASSERT(!iters1.empty());
            IPL_ASSERT(!iters2.empty());
            merge(iters1.begin(), iters1.end(), iters2.begin(), iters2.end(),
                  back_inserter(itersMerged), IterSorter());
            IPL_ASSERT(itersMerged.size() > 1);

            auto t = itersMerged.begin(),
                te = itersMerged.end();
            IPL_ASSERT(t != te);
            while (t != te) {
                N32 xs = (*t)->start().x_,
                    xe = xs + (*t)->len();
                ++t;
                while (t != te && merge(xe, **t))
                    ++t;
                reg.add(Rbo(PointN16(xs,y), xe-xs));
            }
        }
    }
    IPL_ASSERT(r == rend || s == send);
    // lower part
    while (r != rend) {
        reg.add(*r);
        ++r;
    }
    while (s != send) {
        reg.add(*s);
        ++s;
    }
    IPL_ASSERT_VALID(reg);
    return reg;
}

IPL_ANON_NS_BEGIN

//! Intesection of two Rbos.
/*! Intersect a Rbo with end point @a xe and a Rbo @a r.
 * We don't need the start point of the first Rbo, but assume, that it is smaller
 * than the end point of the second Rbo. Additionally we update the end point
 * @a xe such that we can reuse it in the next iteration.
 * @sa Region::intersect
 * @return Length of the intersection or -1 if the two Rbo's are disjunct.
 */
inline N32
intersection(N32 & xe, Rbo const & r)
{
    N32 xs = r.start().x_;
    N32 len = r.len();
    N32 xt = xs + len;

    if (xe <= xs) {
        xe = r.start().x_ + r.len();
        return -1;
    }
    if (xe < xt) {
        len = xe - r.start().x_;
        xe = r.start().x_ + r.len();
    }
    return len;
}

IPL_ANON_NS_END

/*! @internal The procedure is the same as in Region::unions, but for two Rbo's
 * in the same row we do not merge, but insert the intersection of the Rbo's.
 *
 * Only rows where both Regions have Rbo's must be considered. For such rows
 * we store the Rbo's sorted in a temporary vector and intersect adjacent elements.
 * We store the end point of the first Rbo in @a xe and check the next Rbo:
 * - if the Rbo has now intersection with the previous one, update @a xe as end
 *   point of the actual Rbo.
 * - if the Rbo is contained in the previous, add it to the resulting set.
 * - else the two Rbo's overlapp. Add the overlapp to the resulting set and update
 *   @a xe to the end point of the actual Rbo.
 * - Repeat this for all Rbo's.
 *
 * @todo Optimize, avoid the temporary vector.
 */
Region const
Region::intersect(Region const & other) const
{
    IPLLOG_INFO(IPL_FNC_NAME << ": " << *this << " and " << other);
    IPL_ASSERT_VALID(*this);
    IPL_ASSERT_VALID(other);
    // empty
    if (this->empty() || other.empty())
        return Region();

    // disjunct bounding boxes
    PointN16 const & ul1 = this->boundingBox().upperLeft();
    PointN16 const & lr1 = this->boundingBox().lowerRight();
    PointN16 const & ul2 = other.boundingBox().upperLeft();
    PointN16 const & lr2 = other.boundingBox().lowerRight();
    if (lr1.x_ < ul2.x_ || lr2.x_ < ul1.x_
        || lr1.y_ < ul2.y_ || lr2.y_ < ul1.y_)
        return Region();

    vector<RboIterator> iters1, iters2, itersMerged;
    Region reg;
    auto r = this->begin(),
        s = other.begin(),
        rend = this->end(),
        send = other.end();
    if (*s < *r) {
        swap(r, s), swap(rend, send);
    }
    IPL_ASSERT(r != rend && s != send);
    N32 const y = s->start().y_;
    while (r != rend && r->start().y_ < y)
        ++r;
    while (r != rend && s != send) {
        if (r->start().y_ < s->start().y_)
            ++r;
        else if (s->start().y_ < r->start().y_)
            ++s;
        else {
            iters1.clear();
            iters2.clear();
            itersMerged.clear();
            N32 const y = r->start().y_;
            IPL_ASSERT(y == s->start().y_);
            IPL_ASSERT(r != rend && s != send);
            do {
                iters1.push_back(r);
            } while (++r != rend && y == r->start().y_);
            do {
                iters2.push_back(s);
            } while (++s != send && y == s->start().y_);
            IPL_ASSERT(!iters1.empty() && !iters2.empty());
            merge(iters1.begin(), iters1.end(), iters2.begin(), iters2.end(),
                  back_inserter(itersMerged), IterSorter());
            IPL_ASSERT(itersMerged.size() > 1);

            auto t = itersMerged.begin(),
                te = itersMerged.end();
            N32 xe = (*t)->start().x_ + (*t)->len();
            ++t;
            do {
                N32 len = intersection(xe, **t);
                if (len > 0)
                    reg.add(Rbo((*t)->start(), len));
            } while(++t != te);
        }
    }
    IPL_ASSERT_VALID(reg);
    IPLLOG_INFO(IPL_FNC_NAME << ": -> " << reg);
    return reg;
}

/*!
 * @internal A-B is A intersect (Complement B)
 * @todo optimize, implement it dircetely.
 */
Region const
Region::subtract(Region const & other) const
{
    IPLLOG_INFO(IPL_FNC_NAME << ": " << *this << " and " << other);
    IPL_ASSERT_VALID(*this);
    IPL_ASSERT_VALID(other);
    if (this->empty() || other.empty()) {
        return *this;
    }
    WinP const & b1 = this->boundingBox();
    WinP const & b2 = other.boundingBox();
    WinP univ(std::min(b1.upperLeft().x_, b2.upperLeft().x_),
              std::min(b1.upperLeft().y_, b2.upperLeft().y_),
              std::max(b1.lowerRight().x_, b2.lowerRight().x_),
              std::max(b1.lowerRight().y_, b2.lowerRight().y_));

    return this->intersect(other.complement(&univ));
}


/*!
 * @internal Iterate over all rows of the universe and cut ot the Rbo's of the
 * Region.
 *
 * @todo Optimize, avoid the temporary Region.
 */
Region const
Region::complement(WinP const * universe /*= 0*/) const
{
    if (universe) {
        IPLLOG_INFO(IPL_FNC_NAME << " for " << *this
                    << " and universe " << *universe);
    } else {
        IPLLOG_INFO(IPL_FNC_NAME << " for " << *this
                    << " and universe bounding box");
    }
    IPL_ASSERT_VALID(*this);

    if (this->empty()) {
        if (!universe)
            throw EmptyRegionError(string(IPL_FNC_NAME) +
                                   ": empty region and empty universe");
        else
            return Region(*universe);
    }

    Region tmp(*this);
    if (!universe)
        universe = &this->boundingBox();
    else
        tmp.clip(*universe);

    Region reg;
    auto r = tmp.begin();
    auto const re = tmp.end();

    PointN16 pt = universe->upperLeft();
    PointN16 const inc = PointN16(0,1);
    N32 const ye = universe->lowerRight().y_,
        wlen = universe->lowerRight().x_ - universe->upperLeft().x_ + 1;

    if (r != re) {
        N32 const ry = r->start().y_;
        while (pt.y_ <= ye && pt.y_ < ry) {
            reg.add(Rbo(pt, wlen));
            pt += inc;
        }
    }
    while (pt.y_ <= ye && r != re) {
        N32 const ry = pt.y_;
        IPL_ASSERT(ry <= r->start().y_);
        if (pt.y_ == r->start().y_) {
            N32 xs = pt.x_;
            N32 len = r->start().x_ - xs;
            if (len > 0)
                reg.add(Rbo(pt, len));
            xs = r->start().x_ + r->len();
            while (++r != re && r->start().y_ == pt.y_) {
                len = r->start().x_ - xs;
                reg.add(Rbo(PointN16(xs, ry), len));
                xs = r->start().x_ + r->len();
            }
            len = pt.x_ + wlen - xs;
            if (len > 0)
                reg.add(Rbo(PointN16(xs, ry), len));
        } else {
                reg.add(Rbo(pt, wlen));
        }
        pt += inc;
    }
    while (pt.y_ <= ye) {
        reg.add(Rbo(pt, wlen));
        pt += inc;
    }
    IPLLOG_INFO(IPL_FNC_NAME << ": -> " << reg);
    IPL_ASSERT_VALID(reg);
    return reg;
}

IPL_NS_END
