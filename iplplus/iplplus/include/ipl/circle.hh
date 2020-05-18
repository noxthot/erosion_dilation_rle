/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2011
 *              all rights reserved
 *
 * $Id: circle.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Header for ipl::Circle
 *
 ********************************************************************/

#ifndef IPL_CIRCLE_HH
#define IPL_CIRCLE_HH

#include "ipl/config.hh"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "ipl/sshape.hh"

IPL_NS_BEGIN

//! Ein Kreis
/*! Der Kreis wird durch seinen Mittelpunkt @a center und den Radius @a r
  charakterisiert.
 */
class Circle : public SShape
{
public:
    /***********************************/
    //! @name Constructors, Destructor und Zuweisung
    //@{

    //! Ctr
    Circle(PointF64 const & center, F64 radius);

    //! CpyCtr
    Circle(Circle const & rhs);

    //! Zuweisung
    Circle & operator=(Circle const & rhs);

    //! Dtr
    virtual ~Circle();
    //@}

    /***********************************/
    //! @name Basis Informationen
    //@{

    virtual F64 area() const;
    virtual F64 circumference() const;

    //! liefert den Radius
    F64 radius() const {
        return radius_;
    }
    //@}

    /***********************************/
    //! @name geometrische Manipulationen
    //@{
    virtual Circle & translate(PointF64 const & v);
    virtual Circle & rotate(Angle const & alpha);
    //! Erzeugt einen neuen um @a v verschobenen Kreis
    Circle const getTranslate(PointF64 const & v) const;
    //! Erzeugt einen neuen um @a alpha gedrehten Kreis
    Circle const getRotate(Angle const & alpha) const;
    //@}

    /***********************************/
    //! @name debug ausgabe
    //@{
    virtual bool validate() const;
    virtual std::ostream & print(std::ostream & os) const;
    //@}

private:
    //! Radius
    F64 radius_;

    /*! @name Serialisierung
     * @sa @ref serialization
     */
    //@{
    friend class boost::serialization::access;
    template<typename Archive>
    void save(Archive & ar, UN32 /*version*/) const {
        ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(SShape)
           << BOOST_SERIALIZATION_NVP(radius_);
    }
    template<typename Archive>
    void load(Archive & ar, UN32 /*version*/) {
        using boost::serialization::make_nvp;
        Circle tmp(PointF64(0,0), 0);
        ar >> make_nvp(BOOST_PP_STRINGIZE(SShape),
                       boost::serialization::base_object<SShape>(tmp))
           >> make_nvp(BOOST_PP_STRINGIZE(radius_), tmp.radius_);
        *this = tmp;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER();
    //@}
};

IPL_NS_END

#endif //IPL_CIRCLE_HH
