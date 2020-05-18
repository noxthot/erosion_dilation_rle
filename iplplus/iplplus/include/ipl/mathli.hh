/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008 - 2012
 *              all rights reserved
 *
 * $Id: mathli.hh 12 2012-09-05 12:17:22Z fesc $
 *
 * @brief  basic mathematical functions
 *
 *********************************************************************/

#ifndef IPL_MATHLI_HH
#define IPL_MATHLI_HH

#include "ipl/config.hh"

#include <cmath>
#include <numeric>
#include <type_traits>
#include <functional>
#include <algorithm>
#include "ipl/type_manip.hh"

IPL_NS_BEGIN

namespace mathli {

/*! @defgroup mathli Free Mathematical Functions and Linear Algebra.
 *
 * This is a layer above @c cmath that abstracts and overloads standard math functions
 * to abstract from the builtin data types.
 *
 * @internal If you need other functions from @c cmath, provide here a short
 * wrapper instead of using @c cmath directly.
 *
 * Sometimes the things are a little bit tricky. C++ overloads the functions from
 * @c math.h in the namespace @c std, but with g++-3.x and N64 there are sometimes
 * conflicts.
 */
//@{

//! The mysterious number
static F64 const Pi =
                static_cast<F64>(3.1415926535897932384626433832795028841971);
//! SquareRoot of 2
static F64 const SqrtTwo =
                static_cast<F64>(1.4142135623730950488016887242096980785696);

//! Rounds and casts to N32
inline N32
roundF(F64 f)
{
    return static_cast<N32>(round(f));
}

//! sign function: returns the sign as N16
inline N16
sign(F64 a)
{
    if( a < 0.0 ){
        return -1;
    }
    else if( a > 0.0 ){
        return 1;
    }
    else{
        return 0;
    }
}

//! @name Square Root
inline F64
sqrt(F64 a)
{
    return std::sqrt(a);
}

//! @name Exponential and Logarithm
//@{
inline F64
exp(F64 a)
{
    return std::exp(a);
}
inline F64
pow(F64 x, F64 y)
{
    return std::pow(x, y);
}

inline F64
log(F64 a)
{
    return std::log(a);
}
inline F64
log10(F64 a)
{
    return std::log10(a);
}
//@}

//! @name Absolute Value
//@{
inline N32
abs(N32 n)
{
    return std::abs(n);
}
inline UN32
abs(UN32 n)
{
    return n;
}
inline N64
abs(N64 n)
{
#if __GNUC__ == 3
    return ::llabs(n);
#elif __GNUC__ == 4
    return std::abs(n);
#elif defined(_MSC_VER)
    return ::_abs64(n);
#else
#   error "Platform not supported"
#endif
}
inline UN64
abs(UN64 n)
{
    return n;
}

inline F64
abs(F64 a)
{
    return std::abs(a);
}
//@}

/*! @name Trigonometric Functions
 * @sa ipl::Angle
 */
//@{
inline F64
toRad(F64 angle_deg)
{
    return angle_deg * Pi / 180.0;
}

inline F64
toDeg(F64 angle_rad)
{
    return angle_rad * 180 / Pi;
}

inline F64
sin(F64 a)
{
    return std::sin(a);
}

inline F64
asin(F64 a)
{
    return std::asin(a);
}

inline F64
cos(F64 a)
{
    return std::cos(a);
}

inline F64
acos(F64 a)
{
    return std::acos(a);
}

inline F64
tan(F64 a)
{
    return std::tan(a);
}

inline F64
atan(F64 a)
{
    return std::atan(a);
}

inline F64
atan2(F64 y, F64 x)
{
    return std::atan2(y, x);
}
//@}

//! Computes the mean of a sequence.
/*! @c *first, the type of the sequence must be numeric.
 * @sa meanStd
 */
template<typename FwdIt>
inline
F64
mean(FwdIt first, FwdIt last)
{
    typedef typename std::iterator_traits<FwdIt>::value_type val_type;
    // try to avoid overflows
    typedef typename std::conditional<std::is_integral<val_type>::value,
                                      N64,
                                      F64>::type SumType;
    IPL_STATIC_ASSERT(std::is_arithmetic<val_type>::value, "invalid type");
    if (first == last) {
        return 0.0;
    }
    SumType s = SumType();
    s = std::accumulate(first, last, s);
    return static_cast<F64>(s)/std::distance(first, last);
}

//! Computes the mean and the standard deviation of a sequence.
/*! @c *first, the type of the sequence must be numeric. On output @a mu is
 * the mean and @a si the deviation.
 * @sa mean
 * @todo Literature
 */
template<typename FwdIt>
inline
void
meanStd(FwdIt first, FwdIt last, F64 & mu, F64 & si)
{
    N32 c = 0;
    mu = 0, si = 0;

    if (first == last) {
        return;
    }
    mu = *first;
    for ( ; first != last; ++first, ++c) {
        F64 tmp = *first - mu;
        mu += tmp/(c+1);
        si += tmp * (*first - mu);
    }
    if (c > 1) {
        si = mathli::sqrt(si / (c-1));
    } else {
        si = 0;
    }
}

//! Computes the median of a sequence.
/*! The median is the element in the middle of the sequence after sorting.
 * If the sequence has an even number of elements, then we do <em>not</em> use
 * the average of those two elements in the middle but the smaller of them.
 * This guarantees that the function always returns an element of the sequence.
 *
 * On an empty sequence the function returns @c T() if @c T denotes the type
 * of the elements of the sequence.
 *
 * @warning This function alters the sequence. Therefore you must save a copy
 * before calling it, if you need the original one.
 *
 * @note This function needs a <em>random access iterator</em>.
 */
template<typename RanIt>
inline
typename std::iterator_traits<RanIt>::value_type
median(RanIt first, RanIt last)
{
    typedef typename std::iterator_traits<RanIt>::value_type      value_type;
    typedef typename std::iterator_traits<RanIt>::difference_type difference_type;
    if (first == last)
        return value_type();
    difference_type const sz = std::distance(first, last),
        mid = sz%2 ? sz/2 : sz/2-1;
    std::nth_element(first, first+mid, last);
    return *(first+mid);
}
//@}

} //namespace mathli

IPL_NS_END

#endif
