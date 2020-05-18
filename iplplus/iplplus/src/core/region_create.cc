/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: region_create.cc 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  create ipl::Region from SShapes
 *
 ********************************************************************/

#include "ipl/region.hh"

#include "ipl/circle.hh"
#include "ipl/ellipse.hh"
#include "ipl/rect.hh"
#include "ipl/iplerr.hh"

#include <map>
#include <algorithm>

IPL_NS_BEGIN

/*****************************************************************************/
//! Helper Class to Generate a Region from an Ellipse/Circle.
class EllipticRegionCreator {
public:
    //! Generate the Region
    void createRegion(Ellipse const & ell, Region & r) const;
protected:
    //! The Class Logger.
    IPL_DECLARE_CLASS_LOGGER;
};

//! Class Logger, the same as for Region.
IPL_DEFINE_CLASS_LOGGER(EllipticRegionCreator,ipl.region);

void
EllipticRegionCreator::createRegion(Ellipse const & ell, Region & r) const
{
    IPL_ASSERT_VALID(ell);
    // empty region?
    if (ell.ra() <= std::numeric_limits<F64>::epsilon()
        || ell.rb() <= std::numeric_limits<F64>::epsilon()) {
        IPLLOG_DEBUG(IPL_FNC_NAME << " -> empty region");
        return;
    }
    N32 const xm = mathli::roundF(ell.center().x_);
    N32 const ym = mathli::roundF(ell.center().y_);
    F64 const u = ell.angle().cos();
    F64 const v = ell.angle().sin();

    IPL_ASSERT(ell.ra() > 1e-6);
    IPL_ASSERT(ell.rb() > 1e-6);

    F64 const rmax = std::max(ell.ra(), ell.rb());
    F64 const l1 = rmax*rmax/(ell.ra()*ell.ra());
    F64 const l2 = rmax*rmax/(ell.rb()*ell.rb());
    F64 const a = l1*u*u + l2*v*v;
    F64 const b = u*v*(l1-l2);
    F64 const c = l1*v*v + l2*u*u;
    N32 const yb = mathli::roundF(mathli::sqrt(a*rmax*rmax/(a*c-b*b)));
    N32 const ymin = ym-yb;
    N32 const ymax = 1+ym+yb;

    IPLLOG_DEBUG(IPL_FNC_NAME << ": y-span: " << ymin << " - " << ymax);
    for (N32 y = ymin; y < ymax; ++y) {
        F64 const Y = F64(y - ym) + 0.25;
        F64 const delta = b*b*Y*Y - a*(c*Y*Y-rmax*rmax);
        if (delta > 0) {
            F64 const sdelta = mathli::sqrt(delta);
            N32 const xmin = xm - mathli::roundF((b*Y+sdelta)/a);
            N32 const xmax = xm - mathli::roundF((b*Y-sdelta)/a);
            r.add(Rbo(PointN16(xmin, y), xmax - xmin + 1));
        }
    }
}

/*****************************************************************************/

// Adapted Scan-Line Polygon Fill Algorithm from
// "Computer Graphics, C Version (2nd Edition)" von Donald Hearn
// und M. Pauline Baker.

//! Helper Class to Generate a Region from a Polygon.
class PolygonRegionCreator
{
private:
    //! Struct representing an Edge of a Polygon.
    struct Edge {
        N16 yLower,     //!< y-coordinate of the (lower) end of the edge
            xIntersect, //!< x-coordinate of the edge in the actual Scan-Line
            dX,         //!< slope of the edge in x-direction
            dY,         //!< slope of the edge in y-direction
            step;       //!< transition of the edge (to left resp. to right)
        N32 error;      //!< actual error of the Bresenham algorithm

        //! Default Ctr
        Edge() : yLower(0), xIntersect(0),
            dX(0), dY(0), step(0), error(0) {}
        //! Ctr
        Edge(N16 y, N16 x, N16 dx, N16 dy) :
            yLower(y), xIntersect(x), dX(dx), dY(dy),
            step(dx < 0 ? -1 : 1), error(0) { IPL_ASSERT(dy >= 0); }
    };

    //! Functor to find the first edge with a greather @a xIntersect member.
    class IsGreater : public std::unary_function<Edge const &, bool> {
        N16 xIntersect_; //!< x-coordinate of the edge
    public:
        //! Ctr
        explicit IsGreater(N16 xIntersect) : xIntersect_(xIntersect) {}
        //! Compare if the x-coordinate of the edge is greater than that of @a e
        bool operator() (Edge const & e) const {
            return  e.xIntersect > xIntersect_; }
    };

    //! Functor to sort by @a xIntersect member in ascending order.
    struct Less :
        public std::binary_function<Edge const &, Edge const &, bool> {
        //! a.xIntersect < b.xIntersect
        bool operator() (Edge const & a, Edge const & b) const {
            return a.xIntersect < b.xIntersect;
        }
    };

    //! Functor to determine if an edge is completely processed.
    /*! i.e. if the actual Scan-Line >= e.yLower for an edge @a e.
     *
     */
    class IsCompleted : public std::unary_function<Edge const &, bool> {
        N16 scan_; //!< y-coordinate of the actual Scan-Line
    public:
        //! Ctr
        explicit IsCompleted(N16 scan) : scan_(scan) {}
        //! Compare
        bool operator() (Edge const & e) const { return  scan_ >= e.yLower; }
    };

    //! Functor to Update @a xIntersect for an edge when iterating over the
    //! Scan-Lines (via Bresenhame)
    struct Update : public std::unary_function<Edge, Edge> {
        //! Update @a e.xIntersect
        Edge operator() (Edge e) const {
            // dX > dY
            if (mathli::abs(e.dX) > e.dY) {
                while (e.error < mathli::abs(e.dX)) {
                    e.xIntersect += e.step;
                    e.error += 2 * e.dY;
                }
                e.error -= 2 * mathli::abs(e.dX);
            }
            else { // dY >= dX
                e.error += 2 * mathli::abs(e.dX);
                if (e.error >= e.dY) {
                    e.xIntersect += e.step;
                    e.error -= 2 * e.dY;
                }
            }
            return e;
        }
    };

    //! \name Helper Functions
    //@{
    void insertEdge(std::list<Edge> & edges, Edge const & edge);
    N16 yNext(N32 i, std::vector<PointN16> const & pts) const;
    N16 yPrev(N32 i, std::vector<PointN16> const & pts) const;
    void makeEdge(PointN16 upper, PointN16 lower, N16 yComp,
        std::map< N16, std::list<Edge> > & edges);
    N16 buildEdgeList(std::vector<PointN16> const & pts,
        std::map< N16, std::list<Edge> > & edges);
    void buildActiveList(N16 scan, std::list<Edge> & active,
        std::map< N16, std::list<Edge> > & edges);
    void fillScan(N16 scan, std::list<Edge> const & active,
        Region & region);
    void updateActiveList(N16 scan, std::list<Edge> & active);
    //! Sort a list of edges in the actual Scan-Line by @a xIntersect.
    void resortActiveList(std::list<Edge> & active) {
         active.sort(Less());
    }
    //@}

public:
    void createRegion(PolygonN16 const & pol, Region & region);
};

//! Insert an edge @a edge into the list @a edges, sorted by @a xIntersect.
void
PolygonRegionCreator::insertEdge(std::list<Edge> & edges, Edge const & edge)
{
    auto itr =
        std::find_if(edges.begin(), edges.end(), IsGreater(edge.xIntersect));

    edges.insert(itr, edge);
}

//! For an Index @a i return the y-coordinate of the next non-horizontal line.
N16
PolygonRegionCreator::yNext(N32 i, std::vector<PointN16> const & pts) const
{
    N32 j = (i + 1 >= static_cast<N32>(pts.size())) ? 0 : i + 1;

    while (pts[i].y_ == pts[j].y_)
        j = (j + 1 >= static_cast<N32>(pts.size())) ? 0 : j + 1;

    return pts[j].y_;
}

//! For an Index return the y-coordinate of the previous non-horizontal line.
N16
PolygonRegionCreator::yPrev(N32 i, std::vector<PointN16> const & pts) const
{
    N32 j = (i - 1 < 0) ? pts.size() - 1 : i - 1;

    while (pts[i].y_ == pts[j].y_)
        j = (j - 1 < 0) ? pts.size() - 1 : j - 1;

    return pts[j].y_;
}

//! Generate a new edge and add it to the list
void
PolygonRegionCreator::makeEdge(
    PointN16 upper, PointN16 lower, N16 yComp,
    std::map< N16, std::list<Edge> > & edges)
{
    const Edge edge(
        (lower.y_ < yComp) ? lower.y_ - 1 : lower.y_, // yLower
        upper.x_,              // xIntersect
        lower.x_ - upper.x_,  // dX
        lower.y_ - upper.y_); // dY

    insertEdge(edges[upper.y_], edge);
}

//! Generate a list of edges from the polygon @a pts.
N16
PolygonRegionCreator::buildEdgeList(
    std::vector<PointN16> const & pts,
    std::map< N16, std::list<Edge> > & edges)
{
    PointN16 pt1 = pts[pts.size() - 1];
    N16 yLast = yPrev(static_cast<N32>(pts.size()) - 1, pts);
    N16 yLowest = pt1.y_;

    for (N32 i = 0; i < static_cast<N32>(pts.size()); ++i) {
        PointN16 pt2 = pts[i];
        if (pt1.y_ != pt2.y_) {    // nonhorizontal line
            if (pt1.y_ > pt2.y_) { // up-going edge
                makeEdge(pt2, pt1, yLast, edges);
                if (pt1.y_ > yLowest)
                    yLowest = pt1.y_;
            }
            else {                   // down-going edge
                makeEdge(pt1, pt2, yNext(i, pts), edges);
                if (pt2.y_ > yLowest)
                    yLowest = pt2.y_;
            }
            yLast = pt1.y_;
        }
        pt1 = pt2;
    }

    return yLowest;
}

//! For the actual Scan-Line generate a list of edges.
void
PolygonRegionCreator::buildActiveList(
    N16 scan, std::list<Edge> & active,
    std::map< N16, std::list<Edge> > & edges)
{
    auto itr = edges.find(scan);

    if (itr != edges.end())
        active.merge(itr->second, Less());
}

//! Generate the Rbo's for the actual Scan-Line.
void
PolygonRegionCreator::fillScan(
    N16 scan, std::list<Edge> const & active, Region & region)
{
    auto itr = active.begin();

    while (itr != active.end()) {

        Edge const & pt1 = *itr;

        if (++itr != active.end()) {

            Edge pt2 = *itr;

            // avoid overlapping Rbo's
            while (++itr != active.end() &&
                itr->xIntersect <= pt2.xIntersect + 1 &&
                ++itr != active.end())
                pt2 = *itr;

            // Add Rbo to the region
            region.add(Rbo(
                PointN16(pt1.xIntersect, scan),
                pt2.xIntersect - pt1.xIntersect + 1));
        }
    }
}

//! Update the list of edges for the actual Scan-Line.
void
PolygonRegionCreator::updateActiveList(N16 scan, std::list<Edge> & active)
{
    // Remove processed edge
    active.remove_if(IsCompleted(scan));
    // Update @a xIntersect for the remaining edges
    std::transform(active.begin(), active.end(), active.begin(), Update());
}

//! Generate a Region from a Polygon @a pol.
void
PolygonRegionCreator::createRegion(PolygonN16 const & pol, Region & region)
{
    if (pol.size() == 0)
        return;

    std::vector<PointN16> pts(pol.begin(), pol.end());

    // special case, only one horizontal line
    auto itrMin = std::min_element(pts.begin(), pts.end()),
        itrMax = std::max_element(pts.begin(), pts.end());
    if (itrMin->y_ == itrMax->y_) {
        region.add(Rbo(PointN16(itrMin->x_, itrMin->y_),
            itrMax->x_ - itrMin->x_ + 1));
        return;
    }

    // Normal flow
    std::map< N16, std::list<Edge> > edges;
    N16 const yLowest = buildEdgeList(pts, edges);

    if (!edges.empty()) {
        std::list<Edge> active;
        for (N16 scan = edges.begin()->first; scan <= yLowest; ++scan) {
            buildActiveList(scan, active, edges);
            if (!active.empty()) {
                fillScan(scan, active, region);
                updateActiveList(scan, active);
                resortActiveList(active);
            }
        }
    }
}

/*****************************************************************************/

Region::Region(WinP const & win)
    : bbox_(win)
{
    IPL_ASSERT_VALID(win);
    IPLLOG_INFO(IPL_FNC_NAME << " from " << win);
    N16 x = win.upperLeft().x_;
    N32 len = win.lowerRight().x_ - x + 1;
    Rbo rbo(PointN16(x, win.upperLeft().y_), len);
    PointN16 v(0,1);
    while (rbo.start().y_ <= win.lowerRight().y_) {
        this->add(rbo);
        rbo.translate(v);
    }
    IPLLOG_INFO(IPL_FNC_NAME << " -> " << *this);
    IPL_ASSERT_VALID(*this);
}

Region::Region(Ellipse const & ell)
{
    IPL_ASSERT_VALID(ell);
    IPLLOG_INFO(IPL_FNC_NAME << " from " << ell);

    EllipticRegionCreator c;
    c.createRegion(ell, *this);

    IPLLOG_INFO(IPL_FNC_NAME << " -> " << *this);
    IPL_ASSERT_VALID(*this);
}

Region::Region(Circle const & cir)
{
    IPL_ASSERT_VALID(cir);
    IPLLOG_INFO(IPL_FNC_NAME << " from " << cir);

    Ellipse ell(cir.center(), cir.radius(), cir.radius());
    EllipticRegionCreator c;
    c.createRegion(ell, *this);

    IPLLOG_INFO(IPL_FNC_NAME << " -> " << *this);
    IPL_ASSERT_VALID(*this);
}

Region::Region(PolygonN16 const & pol)
{
    IPL_ASSERT_VALID(pol);
    IPLLOG_INFO(IPL_FNC_NAME << " from " << pol);

    PolygonRegionCreator().createRegion(pol, *this);

    IPLLOG_INFO(IPL_FNC_NAME << " -> " << *this);
    IPL_ASSERT_VALID(*this);
}

Region::Region(Rect const & rec)
{
    IPL_ASSERT_VALID(rec);
    IPLLOG_INFO(IPL_FNC_NAME << " from " << rec);

    PointF64 szh = rec.size();
    szh *= 0.5f;

    PointF64  rectCorners[4];
    rec.corners(rectCorners);

    PolygonN16 pol;
    for(N32 i=0; i<4; ++i)
    {
        pol.push_back(rectCorners[i].as<N16>());
    }

    PolygonRegionCreator().createRegion(pol, *this);
    IPL_ASSERT_VALID(*this);
}

IPL_NS_END
