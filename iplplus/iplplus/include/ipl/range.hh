/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Feb 11 2009
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: range.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Miniklasse für einen Bereich
 *
 ********************************************************************/

#ifndef IPL_BOUNDS_HH
#define IPL_BOUNDS_HH

#include "ipl/config.hh"

#include <iostream>

#include "ipl/validable.hh"

IPL_NS_BEGIN

//! An interval
/*! This class is mostly used to handle the allowed range of a parameter.
 * @ingroup HelperTemplates
 */
template<typename T>
struct Range {
    //! Type of the value
    typedef T value_type;
    //! ctr
    Range()
        : min(T()),
          max(T())
    {}
    //! Ctr for a interval of the form @a [min,max]
    Range(T mi, T ma)
        : min(mi),
          max(ma)
    {}
    //! Consistency check
    /*! Mostly for internal use
     */
    bool validate() const {
        return this->min <= this->max;
    }
    //! Check if @a this includes the Range @a rhs.
    bool includes(Range const & rhs) const {
        return this->min <= rhs.min && this->max >= rhs.max;
    }
    //! Check if @a this includes the value @a p.
    template<typename U>
    bool includes(U p) const {
        return p >= this->min && p <= this->max;
    }
    //! Check if the open interval includes the value @a p.
    template<typename U>
    bool includesOpen(U p) const {
        return p > this->min && p < this->max;
    }
    //! Clips @a this to the range of @a rhs.
    Range const & clipTo(Range const & rhs) {
        IPL_ASSERT_VALID(*this);
        IPL_ASSERT_VALID(rhs);
        this->min = std::max(this->min, rhs.min);
        this->max = std::min(this->max, rhs.max);
        return *this;
    }
    //! Clips @a val to the range.
    template<typename U>
    U clip(U val) const {
        IPL_ASSERT_VALID(*this);
        if (val < this->min)
            return this->min;
        else if (val > this->max)
            return this->max;
        else
            return val;
    }
    //! Computes the representant in the interval.
    /*! Computes the representant of a modulus in the range.
     *
     * @pre @a 0 \< @a modulus \< <em>max - min + 1</em>, this ensures that the
     * representant exists and is unique.
     *
     * @param v number for which to calculate the representant.
     * @param modulus modulus value (period).
     * @return representant in the range
     */
    T representant(T v, T modulus) const {
        IPL_ASSERT_VALID(*this);
        IPL_ASSERT(modulus > T()); // ==> modulus > 0
        while (v < min)
            v += modulus;
        while (v > max)
            v -= modulus;
        IPL_ASSERT(v >= min && v <= max);
        return v;
    }
    //! lower bound
    value_type min;
    //! upper bound
    value_type max;
};

//! Debug output into the stream @a os
/*! @relates Range
 */
template<typename T>
inline
std::ostream &
operator<<(std::ostream & os, Range<T> const & r) {
    return os << "[" << r.min << "," << r.max << "]";
}
IPL_NS_END

#endif //IPL_BOUNDS_HH
