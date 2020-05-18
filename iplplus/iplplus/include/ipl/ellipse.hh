/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2011
 *              all rights reserved
 *
 * $Id: ellipse.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Header for ipl::Ellipse
 *
 ********************************************************************/

#ifndef IPL_ELLIPSE_HH
#define IPL_ELLIPSE_HH

#include "ipl/config.hh"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "ipl/sshape.hh"

IPL_NS_BEGIN

//! Eine Ellipse
/*! Die Ellipse wird durch ihre beiden Halbachsen @a ra und @a rb und den
 * Winkel @a alpha, den @a ra mit der @a x - Achse einschließt, beschrieben.
 */
class Ellipse : public SShape
{
public:
    /***********************************/
    //! @name Constructors, Destructor und Zuweisung
    //@{

    //! Ctr
    /*! @a ra ist die Halbachse, die die Richtung von @a alpha hat, @a rb die
     * senkrechte dazu. @a alpha ist der Winkel, den die erste Halbachse mit der
     * @a x - Achse einschließt
     */
    Ellipse(PointF64 const & center,
            F64 ra, F64 rb, Angle const & alpha = Angle());

    //! CpyCtr
    Ellipse(Ellipse const & rhs);

    //! Zuweisung
    Ellipse & operator=(Ellipse const & rhs);

    //! Dtr
    virtual ~Ellipse();
    //@}

    /***********************************/
    //! @name Basis Informationen
    //@{

    virtual F64 area() const;

    //! Perimeter of the Ellipse
    /*! We use the Ramanujan approximation to avoid the evaluation of an elliptic
     * integral
     */
    virtual F64 circumference() const;

    //! liefert die erste Halbachse
    F64 ra() const {
        return ra_;
    }

    //! liefert die zweite Halbachse
    F64 rb() const {
        return rb_;
    }
    //! liefert den Winkel der Ellipse in rad
    Angle const & angle() const {
        return alpha_;
    }
    //@}

    /***********************************/
    //! @name geometrische Manipulationen
    //@{
    virtual Ellipse & translate(PointF64 const & v);
    virtual Ellipse & rotate(Angle const & alpha);
    //! Erzeugt eine neue um @a v verschobene Ellipse
    Ellipse const getTranslate(PointF64 const & v) const;
    //! Erzeugt eine neue um @a alpha gedrehte Ellipse
    Ellipse const getRotate(Angle const & alpha) const;
    //@}

    /***********************************/
    //! @name debug ausgabe
    //@{
    virtual bool validate() const;
    virtual std::ostream & print(std::ostream & os) const;
    //@}

private:

    //! Erste Halbachse
    F64 ra_;

    //! Zweite Halbachse
    F64 rb_;

    //! Richtungswinkel von @a ra_
    Angle alpha_;

    /*! @name Serialisierung
     * @sa @ref serialization
     */
    //@{
    friend class boost::serialization::access;
    template<typename Archive>
    void save(Archive & ar, UN32 /*version*/) const {
        ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(SShape)
           << BOOST_SERIALIZATION_NVP(ra_)
           << BOOST_SERIALIZATION_NVP(rb_)
           << BOOST_SERIALIZATION_NVP(alpha_);
    }
    template<typename Archive>
    void load(Archive & ar, UN32 /*version*/) {
        using boost::serialization::make_nvp;
        Ellipse tmp(*this);
        ar >> make_nvp(BOOST_PP_STRINGIZE(SShape),
                     boost::serialization::base_object<SShape>(tmp))
           >> make_nvp(BOOST_PP_STRINGIZE(ra_), tmp.ra_)
           >> make_nvp(BOOST_PP_STRINGIZE(rb_), tmp.rb_)
           >> make_nvp(BOOST_PP_STRINGIZE(alpha_), tmp.alpha_);
        *this = tmp;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER();
    //@}
};

IPL_NS_END

#endif //IPL_ELLIPSE_HH
