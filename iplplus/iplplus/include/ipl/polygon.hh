/*****************************************************************//**
 *
 * @file
 * @author felix schwitzer
 * @date   Apr 16 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: polygon.hh 11 2012-09-05 12:13:27Z fesc $
 *
 * @brief Header for class Polygon
 *
 ********************************************************************/

#ifndef IPL_POLYGON_HH
#define IPL_POLYGON_HH

#include "ipl/config.hh"

#include <list>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>

#include "ipl/sshape.hh"
#include "ipl/point.hh"
#include "ipl/cache.hh"
#include "ipl/iplerr.hh"

IPL_NS_BEGIN

template<typename T> class Polygon;
class Trafo2D;

//! @name Explicit Instatiations
/*! @relates Polygon
 */
//@{
//! Shortcut to the explicit instatiated Types.
typedef Polygon<N16> PolygonN16;
typedef Polygon<N32> PolygonN32;
typedef Polygon<F64> PolygonF64;
//@}

//! A Polygon.
/*! Polygons are closed paths. The points in the path are stored in a list
 * of points, where adjacent points are interpreted as a line in the path, this
 * holds also for the pair last point - first point. This last pair guarantees
 * that the path is closed.
 *
 * @note At the moment there are explicit instantiations for PointN16, PointN32
 * and PointF64.
 *
 * @section polygon_iterator Iteration over all Points.
 *
 * The class provides forward- and backward- @a iterators, such iterators allow
 * to iterate over all points in a STL compatible way.
 * @code
 *    Polygon<N32> poly;
 *
 *    // fill in some points
 *    for (N32 i = 0; i < 10; ++i) {
 *        poly.push_back(Point<N32>(-i,i));
 *    }
 *
 *    // plot all points
 *    for (auto & p : poly) {
 *        plotPixel(p.x_, p.y_);
 *    }
 *    // iterate over all points in reverse order and translate the point
 *    Point<N32> v(5, 7);
 *    auto pr = poly.rbegin(), pe = poly.rend();
 *    for ( ; pr != pe; ++pr)
 *        *pr += v;
 * @endcode
 *
 * Often you want iterate cyclically over all points in the polygon, i.e to draw
 * the polygon, connecting the adjacent points with lines. For such purposes
 * the class provides @a WrappedIterator.
 * This iterator is a iterator that cycles through the points; when reaching the
 * end of the point sequence it wraps back to the beginning. So be carefull to
 * avoid an endless loop.
 *
 * @code
 *    PolygonN32 pol;
 *    pol.push_back(PointN32(100,100));
 *    pol.push_back(PointN32(500,120));
 *    pol.push_back(PointN32(500,480));
 *    pol.push_back(PointN32(120,500));
 *    auto i = pol.wrappedBegin(), e = i, j = i++;
 *    CImgBoard board;
 *    board.lineColor(Board::Blue)
 *        .opacity(0.5)
 *        .fillColor(Board::Magenta);
 *    board.drawCircle(Circle(*j, 5));
 *    board.lineColor(Board::Red);
 *    do {
 *        board.drawLine(*j, *i);
 *        ++j, ++i;
 *    } while (j != e);
 * @endcode
 */
template<typename T>
class Polygon : public SShape
{
private:
    //! Container for the points
    typedef std::list<Point<T> > cont_type;

public:

    //! @name STL-typedefs
    //@{
    typedef typename cont_type::size_type size_type;
    typedef typename cont_type::difference_type difference_type;
    typedef typename cont_type::reference reference;
    typedef typename cont_type::const_reference const_reference;
    typedef typename cont_type::value_type value_type;
    //@}

    //! forward iterator
    typedef typename cont_type::iterator Iterator;

    //! forward iterator, const version
    typedef typename cont_type::const_iterator ConstIterator;

    //! backward iterator
    typedef typename cont_type::reverse_iterator RevIterator;

    //! backward iterator, const version
    typedef typename cont_type::const_reverse_iterator ConstRevIterator;

    //! cyclic iterator
    /*! @sa wrappedBegin
     */
    class WrappedIterator {
        ///@cond developer_docu
    public:
        //! Have the same traits as for a normal iterator.
        //@{
        typedef typename Iterator::iterator_category iterator_category;
        typedef typename Iterator::value_type        value_type;
        typedef typename Iterator::difference_type   difference_type;
        typedef typename Iterator::pointer           pointer;
        typedef typename Iterator::reference         reference;
        //@}
        //! Dereferencing
        typename cont_type::reference operator*() const {
            return it_.operator*();
        }
        //! Member Access
        typename cont_type::pointer operator->() const {
            return it_.operator->();
        }
        //! Increment
        WrappedIterator & operator++() {
            IPL_ASSERT(it_ != pc_->end());
            ++it_;
            if (it_ == pc_->end())
                it_ = pc_->begin();
            return *this;
        }
        //! Postfix-Increment
        WrappedIterator operator++(int) {
            WrappedIterator tmp(*this);
            ++*this;
            return tmp;
        }
        //! Decrement
        WrappedIterator & operator--() {
            IPL_ASSERT(it_ != pc_->end());
            if (it_ == pc_->begin())
                it_ = pc_->end();
            --it_;
            return *this;
        }
        //! Postfix-Decrement
        WrappedIterator operator--(int) {
            WrappedIterator tmp(*this);
            --*this;
            return tmp;
        }
        //! Inequality
        bool operator!=(WrappedIterator const & rhs) const {
            return this->it_ != rhs.it_;
        }
        //! Equality
        bool operator==(WrappedIterator const & rhs) const {
            return this->it_ == rhs.it_;
        }
    private:
        friend class Polygon<T>;
        //! The base iterator
        Iterator it_;
        //! The associated container
        cont_type * pc_;
        //! Ctr, private, only the class Polygon needs access.
        WrappedIterator(Iterator it, cont_type * pc)
            : it_(it), pc_(pc)
        {}
        ///@endcond
    };
    //! Wrapped Iterator, const version
    /*! @sa wrappedBegin
     */
    class ConstWrappedIterator {
        ///@cond developer_docu
    public:
        //! Have the same traits as for a normal iterator.
        //@{
        typedef typename ConstIterator::iterator_category iterator_category;
        typedef typename ConstIterator::value_type        value_type;
        typedef typename ConstIterator::difference_type   difference_type;
        typedef typename ConstIterator::pointer           pointer;
        typedef typename ConstIterator::reference         reference;
        //@}
        //! Dereferencing
        typename cont_type::const_reference operator*() const {
            return it_.operator*();
        }
        //! Member Access
        typename cont_type::const_pointer operator->() const {
            return it_.operator->();
        }
        //! Increment
        ConstWrappedIterator & operator++() {
            IPL_ASSERT(it_ != pc_->end());
            ++it_;
            if (it_ == pc_->end())
                it_ = pc_->begin();
            return *this;
        }
        //! Postfix-Increment
        ConstWrappedIterator operator++(int) {
            ConstWrappedIterator tmp(*this);
            ++*this;
            return tmp;
        }
        //! Decrement
        ConstWrappedIterator & operator--() {
            IPL_ASSERT(it_ != pc_->end());
            if (it_ == pc_->begin())
                it_ = pc_->end();
            --it_;
            return *this;
        }
        //! Postfix-Decrement
        ConstWrappedIterator operator--(int) {
            ConstWrappedIterator tmp(*this);
            --*this;
            return tmp;
        }
        //! Inequality
        bool operator!=(ConstWrappedIterator const & rhs) const {
            return this->it_ != rhs.it_;
        }
        //! Equality
        bool operator==(ConstWrappedIterator const & rhs) const {
            return this->it_ == rhs.it_;
        }
    private:
        friend class Polygon<T>;
        //! The base iterator
        ConstIterator it_;
        //! The associated container
        cont_type const * pc_;
        //! Ctr, private, only the class Polygon needs access.
        ConstWrappedIterator(ConstIterator it, cont_type const * pc)
            : it_(it), pc_(pc)
        {}
        ///@endcond
    };

    /**************************************************************************/
    //! @name Constructors, Destructor and Assignment
    //@{

    //! Ctr.
    /*! Construct a empty polygon.
     */
    Polygon();

    //! Ctr
    /*! Generate an polygon from a sequence <em>[first, last)</em>. If you have
     * a vector of points, you get the corresponding polygon by
     * @code
     * vector<PointN16> v = ...;
     * Polygon<F64> poly(v.begin(), v.end());
     * @endcode
     *
     * @note This constructor may be used to convert a polygon from one point-type
     * to another.
     * @code
     * PolygonN16 pn;
     * p1.push_back(PointN16());
     * // fill in more points
     * // ...
     * PolygonF64 pf(pn.begin(), pn.end());
     * @endcode
     */
    template<typename InputIterator>
    Polygon(InputIterator first,
            InputIterator last)
        : SShape(PointF64()),
          points_(first, last),
          gravity_(),
          sigArea_()
    {
        IPL_ASSERT_VALID(*this);
    }

    //! CpyCtr
    /*! @todo default */
    //Polygon(Polygon const & rhs);

    //! Assignment
    /*! @todo default */
    //Polygon & operator=(Polygon const & rhs);

    //! Assignment from a sequence.
    /*! Replaces the object with copies of those in [first,last), therefore
     * @c *first must be convertible to @c Point<T>
     */
    template<typename InputIterator>
    void assign(InputIterator first, InputIterator last)
    {
        points_.assign(first, last);
        invalidateCache();
        IPL_ASSERT_VALID(*this);
    }
    //@}

    /**************************************************************************/
    //! @name Iterators
    //@{

    //! Returns the start of the point sequence.
    /*! @sa @ref polygon_iterator "Iteration over all points"
     */
    Iterator begin() {
        invalidateCache();
        return points_.begin();
    }

    //! Returns the end of the point sequence
    /*! @sa @ref polygon_iterator "Iteration over all points"
     */
    Iterator end() {
        invalidateCache();
        return points_.end();
    }

    //! Returns the start of the point sequence.
    /*! @sa @ref polygon_iterator "Iteration over all points"
     */
    ConstIterator begin() const {
        return points_.begin();
    }

    //! Returns the end of the point sequence
    /*! @sa @ref polygon_iterator "Iteration over all points"
     */
    ConstIterator end() const {
        return points_.end();
    }

    //! Returns the start of the point sequence.
    /*! This iterator runs in reverse order
     * @sa @ref polygon_iterator "Iteration over all points"
     */
    RevIterator rbegin() {
        invalidateCache();
        return points_.rbegin();
    }

    //! Returns the end of the point sequence.
    /*! This iterator runs in reverse order
     * @sa @ref polygon_iterator "Iteration over all points"
     */
    RevIterator rend() {
        invalidateCache();
        return points_.rend();
    }

    //! Returns the start of the point sequence.
    /*! This iterator runs in reverse order
     * @sa @ref polygon_iterator "Iteration over all points"
     */
    ConstRevIterator rbegin() const {
        return points_.rbegin();
    }

    //! Returns the end of the point sequence.
    /*! This iterator runs in reverse order
     * @sa @ref polygon_iterator "Iteration over all points"
     */
    ConstRevIterator rend() const {
        return points_.rend();
    }

    //! Returns a wrapped iterator to the first point.
    /*! @sa @ref polygon_iterator "Iteration over all points"
     */
    WrappedIterator const wrappedBegin();

    //! Returns a wrapped iterator to the first point.
    /*! @sa @ref polygon_iterator "Iteration over all points"
     */
    ConstWrappedIterator const wrappedBegin() const;
    //@}

    /**************************************************************************/
    //! @name Basis Information.
    //@{

    //! The number of points in the sequence.
    N32 size() const {
        return points_.size();
    }
    //! Check if the Polygon is nonempty
    bool empty() const {
        return points_.empty();
    }

    //! Calculate the axes parallel Bounding Box.
    /*! The Result is stored in @a ul (the upper left corner) and @a lr (the
     * lower right corner) in the standard coordinate system.
     */
    void boundingBox(Point<T> & ul, Point<T> & lr) const
    {
        Polygon<T>::boundingBox(this->begin(), this->end(), ul, lr);
    }

    //! Compute the Bounding Box of a Point Set.
    /*! Computes the axis parallel bounding box of the Sequence
     * @a [first, last). The sequence must not be empty and <em>*first</em>
     * must be convertible to <em>Point<T></em>.
     *
     * @sa Polygon::boundingBox
     */
    template<typename FwdIt>
    static void
    boundingBox(FwdIt first, FwdIt last, Point<T> & ul, Point<T> & lr)
    {
        typedef typename std::iterator_traits<FwdIt>::value_type::value_type U;
        if (first == last) {
            throw EmptyRegionError("empty polygon");
        }
        U minx = first->x_, maxx = minx;
        U miny = first->y_, maxy = miny;
        ++first;
        for ( ; first != last; ++first) {
            if (first->x_ < minx) {
                minx = first->x_;
            } else if (first->x_ > maxx) {
                maxx = first->x_;
            }
            if (first->y_ < miny) {
                miny= first->y_;
            } else if (first->y_ > maxy){
                maxy = first->y_;
            }
        }
        IPL_ASSERT(minx <= maxx);
        IPL_ASSERT(miny <= maxy);
        ul = Point<U>(minx, miny);
        lr = Point<U>(maxx, maxy);
    }

    //! Compute the (orientated) Area of the Polygon.
    /*! The area can have a sign. The sign of the area depends on the orientation
     * of the polygon. If we run clockwise through the points, then the area
     * is positive, else negative. This can be used as a fast test for the
     * orientation of a polygon.
     * @sa area
     */
    F64 signedArea() const;

    //! Calculates the Winding Number of this Polygon around the Point @a p
    /*! The number is positive, if the polygon is traveled clockwise and
     * negative, if counter-clockwise; if the point lies outside the polygon, the
     * winding number is 0.
     * @par Literature:
     *     http://softsurfer.com/Archive/algorithm_0103/algorithm_0103.htm
     */
    template <typename U>
    N32 winding(Point<U> const & p) const
    {
        return Polygon<T>::winding(p, this->begin(), this->end());
    }

    //! Calculates the Winding Number of a Curve around the Point @a p
    /*! The curve is represented as a sequence of the points [first, last) and
     * assumed to be closed.
     * @sa Polygon::winding
     */
    template<typename BidIt, typename U>
    static N32
    winding(Point<U> const & p, BidIt first, BidIt last)
    {
        N32 wn = 0;
        BidIt i = last, j = first;
        // loop through all edges of the polygon
        --i;
        while (j != last) {
            if (i->y_ <= p.y_) { // an upward crossing
                if (j->y_ > p.y_)
                    if (isLeft(*i, *j, p) > 0)
                        ++wn;// have a valid up intersect
            } else {
                if (j->y_ <= p.y_) // a downward crossing
                    if (isLeft(*i, *j, p) < 0)
                        --wn;// have a valid down intersect
            }
            i = j++;
        }
        return wn;
    }

    //! Calculate the Area of a Polygon.
    /*! @sa signedArea
     */
    virtual F64 area() const;
    virtual F64 circumference() const;

    virtual PointF64 const & center() const;
    //@}

    /**************************************************************************/
    //! @name Insertion and Deletion of Points.
    //@{

    //! Append at the End.
    void push_back(Point<T> const & p);

    //! Append at the Beginning.
    void push_front(Point<T> const & p);

    //! Insert the Elementes @a first through @a last before Position @a pos.
    template<typename InputIterator>
    void insert(Iterator pos, InputIterator first, InputIterator last) {
        invalidateCache();
        points_.insert(pos, first, last);
        IPLLOG_DEBUG(IPL_FNC_NAME << ": add "
            << std::distance(first, last) << " point(s)");
    }

    //! Insert an Element @a p before @a pos.
    /*! Returns the iterator to the inserted element.
     * @sa insertAfter
     */
    Iterator insert(Iterator pos, Point<T> const & p) {
        invalidateCache();
        return points_.insert(pos, p);
    }

    //! Insert an Element @a p @a pos,
    /*! Returns the iterator to the inserted element.
     * @sa insert
     */
    Iterator insertAfter(Iterator pos, Point<T> const & p) {
        invalidateCache();
        return points_.insert(pos++, p);
    }

    //! Deletes the last Point.
    void pop_back() {
        invalidateCache();
        points_.pop_back();
    }

    //! Deletes the Point at Position @a pos.
    /*! Returns the iterator pointing to the next element or @c end if there are
     * no more elements.
     * @sa clear
     */
    Iterator erase(Iterator pos) {
        invalidateCache();
        return points_.erase(pos);
    }

    //! Deletes all Points in the Polygon.
    /*! After this operation the polygon will be empty.
     * @sa erase
     */
    void clear() {
        invalidateCache();
        points_.clear();
    }

    //! Remove equal adjacent Points.
    /*! If you insert such double points or an other operation produces such
     * points, then the sequence contains unnecessary points. The object is
     * a valid one, but we can remove the spurious points with this method.
     */
    Polygon & removeDoublePoints();
    //@}

    /**************************************************************************/
    //! @name Geometric Manipulations.
    //@{

    //! Translates the Polygon by a Vector @a v.
    virtual Polygon & translate(PointF64 const &);

    //! Rotates the Polygon by an Angle @a alpha around its Eenter.
    virtual Polygon & rotate(Angle const &);

    //! Transforms the Polygon by a Tranformation @a tr.
    Polygon & transform(Trafo2D const & tr);

    //! Create a new, translated Polygon.
    Polygon<T> const getTranslate(PointF64 const & v) const;

    //! Create a new, rotated Polygon.
    Polygon<T> const getRotate(Angle const & alpha) const;

    //! Create a new, transformed Polygon.
    Polygon<T> const getTransform(Trafo2D const & tr) const;
    //@}

    /****************************************************************/
    //! @name Debug Output.
    //@{
    virtual bool validate() const;
    virtual std::ostream & print(std::ostream & os) const;
    //@}

protected:
    //! The logger
    IPL_DECLARE_CLASS_LOGGER;

private:

    //! Container with the Point.
    cont_type points_;

    //! Center of Gravity
    /*! @internal Makes the member @c center_ superfluous
     */
    mutable Cache<PointF64> gravity_;

    //! The signed area
    mutable Cache<F64> sigArea_;

    //! Invalidates the cached data
    void invalidateCache();

    //! Helper function for winding
    /*! @return
     *  @retval >0 if p2 left of the line p0-p1
     *  @retval =0 if p2 on the line
     *  @retval <0 if p2 right of the line
     */
    template<typename U>
    static F64
    isLeft(Point<T> const & p0, Point<T> const & p1, Point<U> const & p2) {
        return (p1.x_ - p0.x_)*(p2.y_ - p0.y_) - (p2.x_ - p0.x_)*(p1.y_ - p0.y_);
    }

    /*! @name Serialization.
     * @sa @ref serialization
     */
    //@{
    friend class boost::serialization::access;
    template<typename Archive>
    void save(Archive & ar, UN32 /*version*/) const {
        ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(SShape)
           << BOOST_SERIALIZATION_NVP(points_);
    }
    template<typename Archive>
    void load(Archive & ar, UN32 /*version*/) {
        using boost::serialization::make_nvp;
        Polygon tmp;
        ar >> make_nvp(BOOST_PP_STRINGIZE(SShape),
                       boost::serialization::base_object<SShape>(tmp))
           >> make_nvp(BOOST_PP_STRINGIZE(points_), tmp.points_);
        *this = tmp;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER();
    //@}
};

IPL_NS_END

#endif //IPL_POLYGON_HH
