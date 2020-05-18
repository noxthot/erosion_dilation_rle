/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Jul 7, 2009
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: trafo2d.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Header for Trafo2D
 *
 ********************************************************************/
#ifndef IPL_TRAFO2D_HH_
#define IPL_TRAFO2D_HH_

#include "ipl/config.hh"

#include <limits>
#include <iosfwd>
#include <boost/array.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "ipl/point.hh"
#include "ipl/angle.hh"
#include "ipl/iplerr.hh"

IPL_NS_BEGIN

//! Affine 2D Transformations
/*! @ingroup mathli
 * This class provides methods to generate 2D affine transformations and to apply
 * them to points.
 *
 * A general affine transform can be composed by concatenating simple
 * transformations. Lets denote a rotation with center @a c and angle @a alpha
 * by @f$ R(\alpha, c) @f$ and a translation by @a v by @f$ t_v @f$, then
 * the following code
 *
 * @code
 * Trafo2D tr;    // identity
 * // rotate at (100,100) by 45 degree
 * tr.rotate(Angle(45, Angle::InDeg), PointF64(100,100));
 * // then translate
 * tr.translate(PointF64(120,-120));
 * // and apply the resulting trafo to a point
 * res = tr.apply(pt);
 * @endcode
 *
 * is exactly the mathematical operation
 * @f[
 *    R(\alpha, c) \cdot t_v \cdot p
 * @f]
 *
 * The transformation is stored as a homogeneous matrix of the form
 * @f[
 *  \begin{pmatrix} a_{00} & a_{01} & v_x \\
 *                  a_{10} & a_{11} & v_y \\
 *                   0  &  0  &  1
 *  \end{pmatrix}
 * @f]
 */
class Trafo2D {
public:
    /**************************************************************************/
    //! @name Constructors, Destructor and Assignment
    //@{

    //! Ctr, constructs the identity
    Trafo2D();

    //! CpyCtr
    /*! @todo default */
    //Trafo2D(Trafo2D const & rhs) = default;

    //! Dtr
    /*! @todo default */
    //~Trafo2D() = default;

    //! Assignment
    /*! @todo default */
    //Trafo2D & operator=(Trafo2D const &) = default;
    //@}

    /**************************************************************************/
    //! @name Manipulations
    //@{

    //! Resets the trafo to the identity
    Trafo2D & reset();

    //! Rotates by an angle @a alpha with center (0,0)
    Trafo2D & rotate(Angle const & alpha);

    //! Rotates by an angle @a alpha with center @a center
    Trafo2D & rotate(Angle const & alpha, PointF64 const & center);

    //! Translates by the vector @a v
    Trafo2D & translate(PointF64 const & v);

    //! Translates by @a v
    /*! This method first translates and then the original trafo is applied,
     * @f$ T \mapsto T \cdot t_v @f$
     */
    Trafo2D & translatePrior(PointF64 const & v);

    //! Scales both axes with an factor @a s with center (0,0)
    Trafo2D & scale(F64 s);

    //! Scales the <em>x-</em> respectively <em>y-</em>axis with @a sx resp. @a sy.
    Trafo2D & scale(F64 sx, F64 sy);

    //! Scales both axes by @a s with center @a center
    Trafo2D & scale(F64 s, PointF64 const & center);

    //! Scales the corresponding axes with center @a center
    Trafo2D & scale(F64 sx, F64 sy, PointF64 const & center);
    //@}

    /**************************************************************************/
    //! @name Apply a Trafo
    //@{
    //! Applies the trafo to a point @a pt and returns the result
    PointF64 const apply(PointF64 const & pt) const;
    PointN32 const apply(PointN32 const & pt) const;
    PointN16 const apply(PointN16 const & pt) const;
    //@}

    /**************************************************************************/
    //! @name Information
    //@{

    //! Returns the determinate of the linear part of the transform
    F64 det() const;

    //! Returns the rotation as Angle
    /*! @exception ProcessingError if the trafo is not a Galilei transformation
     * (rigid body transformation).
     */
    Angle const angle() const {
        if (this->galileian()) {
            return Angle::atan2(a10_, a00_);
        } else {
            throw ProcessingError("not a rigid-body tranformation");
        }
    }

    //! Returns the translation part as PointF64.
    PointF64 const translation() const {
        return PointF64(vx_, vy_);
    }

    //! Returns the linear part of the transformation.
    /*! This method returns a new transformation containing only the linear part
     * of the original transformation.
     */
    Trafo2D const linearPart() const;

    //! Signalizes if the Trafo is galileian (rigid body Trafo)
    bool galileian() const;

    //! Read access to the elements of the homogeneous matrix
    //@{
    F64 a00() const { return a00_; }
    F64 a01() const { return a01_; }
    F64 a10() const { return a10_; }
    F64 a11() const { return a11_; }
    F64 vx()  const { return vx_;  }
    F64 vy()  const { return vy_;  }
    //@}
    //@}

    /**************************************************************************/
    //! @name Composition
    //@{

    //! Returns the Inverse of the Object as a new Transformation
    Trafo2D const inv() const;

    //! Composition
    /*! Returns the right-composition of \a object with \a rhs. Thus for
     * <tt>PointF64 p</tt> the statement <tt>(t1*t2).apply(p)</tt> is equivalent
     * to <tt>t1.apply(t2.apply(p))</tt>
     */
    Trafo2D const operator*(Trafo2D const & rhs) const;

    //! Composition
    /*! Replaces the object with @a obj*rhs
     */
    Trafo2D & operator*=(Trafo2D const & rhs);
    //@}

    //! Computes the mapping between two triangles.
    /*! Computes a transformation @a tr, such that <em>p -> pNew</em>,
     * <em>q -> qNew</em> and <em>r -> rNew</em> holds.
     * @throw ProcessingError, if the triple <em>(p,q,r)</em> is collinear or
     *    the resulting transform is singular.
     */
    static Trafo2D const fromTriangles(PointF64 const & p,
                                       PointF64 const & q,
                                       PointF64 const & r,
                                       PointF64 const & pNew,
                                       PointF64 const & qNew,
                                       PointF64 const & rNew);

    //! Computes the rigid body transform from a translation and an angle
    /*! Translates @a from into the target point @a to and rotates by
     * @a angle
     */
    static Trafo2D const fromPoses(PointF64 const & from,
                                   PointF64 const & to,
                                   Angle const & angle = Angle());
private:
    //! linear part
    //@{
    F64 a00_, a01_;
    F64 a10_, a11_;
    //@}
    //! translation
    F64 vx_;
    //! translation
    F64 vy_;

    /*! @name Serialization
     * @sa @ref serialization
     */
    //@{
    friend class boost::serialization::access;
    template<typename Archive>
    void save(Archive & ar, UN32 /*version*/) const {
        ar << BOOST_SERIALIZATION_NVP(a00_)
           << BOOST_SERIALIZATION_NVP(a01_)
           << BOOST_SERIALIZATION_NVP(a10_)
           << BOOST_SERIALIZATION_NVP(a11_)
           << BOOST_SERIALIZATION_NVP(vx_)
           << BOOST_SERIALIZATION_NVP(vy_);
    }
    template<typename Archive>
    void load(Archive & ar, UN32 /*version*/) {
        using boost::serialization::make_nvp;
        Trafo2D tmp;
        ar >> make_nvp(BOOST_PP_STRINGIZE(a00_), tmp.a00_)
           >> make_nvp(BOOST_PP_STRINGIZE(a01_), tmp.a01_)
           >> make_nvp(BOOST_PP_STRINGIZE(a10_), tmp.a10_)
           >> make_nvp(BOOST_PP_STRINGIZE(a11_), tmp.a11_)
           >> make_nvp(BOOST_PP_STRINGIZE(vx_), tmp.vx_)
           >> make_nvp(BOOST_PP_STRINGIZE(vy_), tmp.vy_);
        *this = tmp;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER();
    //@}
};

//! @name Debug Output
//@{
/*! @relates Trafo2D
 */
std::ostream &
operator<<(std::ostream & os, Trafo2D const & tr);
//@}

IPL_NS_END

#endif // IPL_TRAFO2D_HH_
