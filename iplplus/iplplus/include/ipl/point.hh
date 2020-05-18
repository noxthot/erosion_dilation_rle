/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: point.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Header for ipl::Point<T>
 *
 ********************************************************************/

#ifndef IPL_POINT_HH
#define IPL_POINT_HH

#include "ipl/config.hh"

#include <iostream>
#include <type_traits>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "ipl/ipltypes.hh"
#include "ipl/validable.hh"
#include "ipl/mathli.hh"

IPL_NS_BEGIN

template<typename T> class Point;

//! @name Explizit Instantiations
//@{
//! @relates Point
//@{
//! shortcut to some Point-templates
typedef Point<N16> PointN16;
typedef Point<N32> PointN32;
typedef Point<F64> PointF64;
//@}
//@}

//! A 2D-point
/*!
 * @tparam T coordinate type, this must be a numerical type
 * @note At the moment there exist explicit instantiations for @c Point<N16>
 *   @c Point<N32> and @c Point<F64>.
 * @sa @ref coordinate_system
 */
template<typename T>
class Point
{
public:
    //! Type of the coordinates @a x and @a y
    typedef T value_type;

    /**************************************************************************/
    //! @name Constructors, Destructor and Assignment
    //@{

    //! Ctr
    explicit Point(T x = T(),
                   T y = T())
        : x_(x),
          y_(y)
    {
        IPL_STATIC_ASSERT(std::is_arithmetic<T>::value,
                          "point needs a numeric type");
    }

    //! CpyCtr
    /*! @todo default */
    //Point(Point const & other) = default;

    //! Templatized CpyCtr
    template<typename U>
    Point(Point<U> const & other)
        : x_(other.x_),
          y_(other.y_)
    {
        IPL_STATIC_ASSERT(convertible_from<U>::value, "invalid point conversion");
    }

    //! Dtr
    /*! @todo default */
    //~Point() = default;

    //! Assignment
    /*! @todo default */
    //Point & operator=(Point const & rhs) = default;

    //! Templatized Assignment
    template<typename U>
    Point & operator=(Point<U> const & rhs) {
        IPL_STATIC_ASSERT(convertible_from<U>::value, "invalid point conversion");
        if (static_cast<void const *>(this) != static_cast<void const *>(&rhs)) {
            x_ = rhs.x_;
            y_ = rhs.y_;
        }
        return *this;
    }
    //@}

    /**************************************************************************/
    //! @name Arithmetics
    //@{

    //! Add @a rhs to this
    Point & operator+=(Point const & rhs) {
        x_ += rhs.x_;
        y_ += rhs.y_;
        return *this;
    }

    //! Subtract @a rhs from this
    Point & operator-=(Point const & rhs) {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        return *this;
    }

    //! unary plus
    Point const & operator+() const {
        return *this;
    }

    //! unary minus
    Point const operator-() const {
        return Point(-x_, -y_);
    }

    //! Scalar product
    template<typename U>
    inline
    auto
    operator*(Point<U> const & rhs) const -> decltype(T(0)*U(0)) {
        return this->x_*rhs.x_ + this->y_*rhs.y_;
    }
    //@}

    /**************************************************************************/
    //! @name Manipulation
    //@{

    //! Returns a new Point with coordinates casted/rounded to @c U
    /*! @note This is a conversion operator, use it with care like static
     *   casts from N32 to N16. When converting from F64 to an integral type,
     *   this function rounds.
     */
    template<typename U>
    Point<U> const as() const;

    //! Scaling
    Point & operator*=(T s) {
        x_ *= s;
        y_ *= s;
        return *this;
    }

    //! Norm, if a Point is identified with a 2D vector.
    F64 norm() const {
        return mathli::sqrt(this->normSq());
    }

    //! Squared norm, if a Point is identified with a 2D vector.
    F64 normSq() const {
        F64 r(x_), s(y_);
        return r*r + s*s;
    }
    //@}

    //! x-coordinate
    T x_;
    //! y-coordinate
    T y_;

    /**************************************************************************/
    //! @name Debug Output
    //@{
    //! debug output
    std::ostream & print(std::ostream & os) const;
    //@}

private:
    /**************************************************************************/
    /*! @name Serialization
     * @sa @ref serialization
     */
    //@{
    friend class boost::serialization::access;
    template<typename Archive>
    void save(Archive & ar, UN32 /*version*/) const {
        ar << BOOST_SERIALIZATION_NVP(x_)
           << BOOST_SERIALIZATION_NVP(y_);
    }
    template<typename Archive>
    void load(Archive & ar, UN32 /*version*/) {
        using boost::serialization::make_nvp;
        Point tmp;
        ar >> make_nvp(BOOST_PP_STRINGIZE(x_), tmp.x_)
           >> make_nvp(BOOST_PP_STRINGIZE(y_), tmp.y_);
        *this = tmp;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER();
    //@}

    //! @cond developer_docu
    /*! A simple trait. The expression @c convertible_from<U>::value evaluates
     * to @c true if and only if @c Point<U> can be converted to @c Point<T>
     * without narrowing the data. Mainly used in static asserts.
     */
    template<typename U>
    struct convertible_from
        : public std::is_same<typename std::common_type<T,U>::type, T>
    {};
    //! @endcond
};

//! @name Arithmetics
//@{

//! Addition
/*! The result type of this operation is analog to the implicit conversion a
 * compiler does.
 * @relates Point
 */
template<typename U, typename V>
inline
Point<typename std::common_type<U,V>::type> const
operator+(Point<U> const & lhs,
               Point<V> const & rhs)
{
    return Point<typename std::common_type<U,V>::type>(lhs.x_ + rhs.x_,
                                                       lhs.y_ + rhs.y_);
}

//! Subtraction
/*! The result type of this operation is analog to the implicit conversion a
 * compiler does.
 * @relates Point
 */
template<typename U, typename V>
inline
Point<typename std::common_type<U,V>::type> const
operator-(Point<U> const & lhs,
               Point<V> const & rhs)
{
    return Point<typename std::common_type<U,V>::type>(lhs.x_ - rhs.x_,
                                                       lhs.y_ - rhs.y_);
}

//! Scalar Multiplication
/*! @relates Point
 */
template<typename T>
inline
Point<T> const
operator *(T v, Point<T> const & p) {
    Point<T> r(p);
    return r *= v;
}
//@}

/*! @name Ordering Relationship
 * This ordering is the induced lexicographical ordering with @a y as the
 * dominant component.
 * @relates Point
 */
//@{
/*! @relates Point
 */
template<typename T, typename U>
inline bool
operator==(Point<T> const & lhs, Point<U> const & rhs) {
    return lhs.x_ == rhs.x_ && lhs.y_ == rhs.y_;
}

/*! @relates Point
 */
template<typename T, typename U>
inline bool
operator!=(Point<T> const & lhs, Point<U> const & rhs) {
    return !(lhs == rhs);
}

/*! @relates Point
 */
template<typename T, typename U>
inline bool
operator<(Point<T> const & lhs, Point<U> const & rhs) {
    return (lhs.y_ < rhs.y_)
        || (!(rhs.y_ < lhs.y_) && lhs.x_ < rhs.x_);
}

/*! @relates Point
 */
template<typename T, typename U>
inline bool
operator<=(Point<T> const & lhs, Point<U> const & rhs) {
    return !(rhs < lhs);
}

/*! @relates Point
 */
template<typename T, typename U>
inline bool
operator>(Point<T> const & lhs, Point<U> const & rhs) {
    return rhs < lhs;
}

/*! @relates Point
 */
template<typename T, typename U>
inline bool
operator>=(Point<T> const & lhs, Point<U> const & rhs) {
    return !(lhs < rhs);
}
//@}

//! @name Debug Output
//@{
/*! @relates Point
 * @sa Point::print
 */
template<typename T>
inline
std::ostream & operator<<(std::ostream & b,
                          Point<T> const & p) {
    return p.print(b);
}
//@}

IPL_NS_END

#endif
