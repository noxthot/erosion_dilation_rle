/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2011
 *              all rights reserved
 *
 * $Id: sshape.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Header for ipl::SShape
 *
 ********************************************************************/

#ifndef IPL_SSHAPE_HH
#define IPL_SSHAPE_HH

#include "ipl/config.hh"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/access.hpp>

#include "ipl/validable.hh"
#include "ipl/point.hh"
#include "ipl/angle.hh"

IPL_NS_BEGIN

//! Basis für geometrische Primitive
/*! Solche Primitive werden durch ihren Typ und den Mittelpunkt/Schwerpunkt
 * gekennzeichnet.
 */
class SShape : public Validable
{
public:

    //! Ctr
    /*! @a center ist der Mittelpunkt des Primitives
     */
    explicit SShape(PointF64 const & center)
        : center_(center)
    {}

    //! CpyCtr
    SShape(SShape const & rhs)
        : Validable(rhs),
          center_(rhs.center())
    {}

    //! Zuweisung
    SShape & operator=(SShape const & rhs) {
        Validable::operator=(rhs);
        if (this != &rhs)
            center_ = rhs.center();
        return *this;
    }

    //! Dtr
    virtual ~SShape()
    {}

    /***********************************/
    //! @name Basis Informationen
    //@{

    //! Liefert die Fläche des Primitives
    virtual F64 area() const = 0;

    //! Liefert den Mittelpunkt des Primitives
    /*! Dies ist normalerweise der Schwerpunkt, um diesen wird gedreht.
     */
    virtual PointF64 const & center() const {
        return center_;
    }

    //! Circumference of the Shape
    virtual F64 circumference() const = 0;
    //@}

    /***********************************/
    //! @name geometrische Manipulationen
    //@{

    //! Verschiebt das Primitiv um den Vektor @a v
    virtual SShape & translate(PointF64 const & v) = 0;

    //! Dreht das Primitiv um alpha um sein Zentrum
    virtual SShape & rotate(Angle const & alpha) = 0;
    //@}

protected:

    //! Mittelpunkt des Primitives
    PointF64 center_;

private:

    /*! @name Serialisierung
     * @sa @ref serialization
     */
    //@{
    // Hier brauche ich für die Exception-Sicherheit nicht splitten, da niemals
    // ein Objekt dieses Typs direkt serialisiert wird
    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive & ar, UN32 /*version*/) {
        ar & BOOST_SERIALIZATION_NVP(center_);
    }
    //@}
};

IPL_NS_END

#endif //IPL_SSHAPE_HH
