/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: angle.hh 8 2012-09-05 11:32:38Z fesc $
 *
 * @brief  Header for Angle
 *
 ********************************************************************/

#ifndef IPL_ANGLE_HH
#define IPL_ANGLE_HH

#include "ipl/config.hh"

#include <iostream>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "ipl/mathli.hh"

IPL_NS_BEGIN

//! An Angle
/*! Encapsulates a value as angle to avoid confusion between the units deg/rad.
 */
class Angle {
public:

    //! Unit Flags
    enum Unit {
        InDeg,
        InRad
    };

    /**************************************************************************/
    //! @name Constructors, Destructor and Assignment
    //@{

    //! DefaultCtr, a angle with 0 deg/rad
    Angle()
        : angle_(0)
    {}

    //! ctr from a value @a v
    /*! Interprets the value @a v depending on the unit @a u
     */
    Angle(F64 v, Unit u)
        : angle_(u==InDeg ? mathli::toRad(v) : v)
    {}

    //! CpyCtr
    /*! @todo default */
    //Angle(Angle const & rhs) = default;

    //! Dtr
    /*! @todo default */
    //~Angle() = default;

    //! Assignment
    /*! @todo default */
    //Angle & operator=(Angle const & rhs) = default;
    //@}

    /**************************************************************************/
    /*! @name Trigonometric Functions
     * This are the standard trigonometric functions bound to an Angle object.
     * @sa mathli
     */
    //@{

    //! Returns the angle in deg
    F64 deg() const {
        return mathli::toDeg(angle_);
    }

    //! Returns the angle in rad
    F64 rad() const {
        return angle_;
    }

    //! sin
    F64 sin() const {
        return mathli::sin(angle_);
    }

    //! cos
    F64 cos() const {
        return mathli::cos(angle_);
    }

    //! tan
    F64 tan() const {
        return mathli::tan(angle_);
    }
    //@}

    /**************************************************************************/
    //! @name Arithmetics
    //@{

    //! unary minus
    Angle const operator-() const {
        Angle ret(*this);
        ret.angle_ = -angle_;
        return ret;
    }

    //! Addition
    Angle & operator+=(Angle const & rhs) {
        angle_ += rhs.angle_;
        return *this;
    }

    //! Addition
    Angle const operator+(Angle const & rhs) const {
        Angle tmp(*this);
        return tmp += rhs;
    }

    //! Subtraction
    Angle & operator-=(Angle const & rhs) {
        angle_ -= rhs.angle_;
        return *this;
    }

    //! Subtraction
    Angle const operator-(Angle const & rhs) const {
        Angle tmp(*this);
        return tmp -= rhs;
    }
    //@}

    /**************************************************************************/
    //! @name Relational Operators
    //@{
    bool operator==(Angle const & rhs) const {
        return angle_ == rhs.angle_;
    }
    bool operator!=(Angle const & rhs) const {
        return !(*this == rhs);
    }
    bool operator<(Angle const & rhs) const {
        return angle_ < rhs.angle_;
    }
    bool operator<=(Angle const & rhs) const {
        return angle_ <= rhs.angle_;
    }
    bool operator>(Angle const & rhs) const {
        return angle_ > rhs.angle_;
    }
    bool operator>=(Angle const & rhs) const {
        return angle_ >= rhs.angle_;
    }
    //@}

    /**************************************************************************/
    /*! @name Inverse Trigonometric Functions.
     * These functions return (in contrast to the standard inverse functions)
     * an @c Angle object.
     * @sa mathli
     */
    //@{

    //! arcsin
    static Angle const
    asin(F64 a) {
        return Angle(mathli::asin(a), InRad);
    }

    //! arccos
    static Angle const
    acos(F64 a) {
        return Angle(mathli::acos(a), InRad);
    }

    //! arctan
    static Angle const
    atan(F64 a) {
        return Angle(mathli::atan(a), InRad);
    }

    //! atan2
    static Angle const
    atan2(F64 y, F64 x) {
        return Angle(mathli::atan2(y, x), InRad);
    }
    //@}

    /**************************************************************************/
    //! @name Debug Output
    //@{
    std::ostream & print(std::ostream & os) const {
        return os << angle_ << "(" << this->deg() << " deg)";
    }
    //@}
private:

    //! the value in rad
    F64 angle_;

    /*! @name Serialization
     * @sa @ref serialization
     */
    //@{
    friend class boost::serialization::access;
    template<typename Archive>
    void save(Archive & ar, UN32 /*version*/) const {
        ar << BOOST_SERIALIZATION_NVP(angle_);
    }
    template<typename Archive>
    void load(Archive & ar, UN32 /*version*/) {
        Angle tmp;
        ar >> boost::serialization::make_nvp(BOOST_PP_STRINGIZE(angle_), tmp.angle_);
        *this = tmp;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER();
    //@}
};

//! Debug Output
inline
std::ostream & operator<<(std::ostream & b,
                          Angle const & p) {
    return p.print(b);
}

IPL_NS_END

#endif //IPL_ANGLE_HH
