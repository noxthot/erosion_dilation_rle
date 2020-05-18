/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2011
 *              all rights reserved
 *
 * $Id: rect.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Header for ipl::Rect
 *
 ********************************************************************/

#ifndef IPL_RECT_HH
#define IPL_RECT_HH

#include "ipl/config.hh"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "ipl/sshape.hh"
#include "ipl/point.hh"
#include "ipl/trafo2d.hh"

IPL_NS_BEGIN

class WinP;

//! Ein evt. gedrehtes Rechteck
/*! Das Rechteck wird charakterisiert durch seinen Mittelpunkt, seine Größe und
 * einen Winkel, wobei die Größe als Punkt ::PointF64 angegeben wird.
 * Die @a x resp. @a y Koordinate beziehen sich dabei auf die Größen, wenn das
 * Rechteck ungedreht ist.
 */
class Rect : public SShape
{
public:
    /***********************************/
    //! @name Constructors, Destructor und Zuweisung
    //@{

    //! Ctr
    Rect(PointF64 const & center,
         PointF64 const & size,
         Angle const & alpha = Angle());

    //! Ctr aus einem WinP
    Rect(WinP const & win);

    //! CpyCtr
    Rect(Rect const & rhs);

    //! Zuweisung
    Rect & operator=(Rect const & rhs);

    //! Dtr
    virtual ~Rect();
    //@}

    /***********************************/
    //! @name Basis Informationen
    //@{

    virtual F64 area() const;
    virtual F64 circumference() const;

    //! liefert die Größe des Rechtecks
    /*! @a x und @a y Werte beziehen sich auf eine Drehlage von 0 deg
     */
    PointF64 const & size() const {
        return size_;
    }

    //! Winkel
    Angle const & angle() const {
        return alpha_;
    }

    //! Liefert die Eckpunkte des Rechteckes
    /*! Die 4 Eckpunkte werden auf den Iterator @a res geschrieben, wobei der erste
     * Punkt der Bildpunkt der linken oberen Ecke ist und das Rechteck dann im
     * Uhrzeigersinn durchlaufen wird. @a *res muss nach @c PointF64 konvertierbar
     * sein.
     */
    template<typename OutIt>
    void corners(OutIt res) const {
        F64 const hx2 = this->size().x_ / 2;
        F64 const hy2 = this->size().y_ / 2;
        Trafo2D trafo;
        trafo.rotate(this->angle(), this->center());
        *res++ = trafo.apply(this->center() + PointF64(-hx2,-hy2));
        *res++ = trafo.apply(this->center() + PointF64( hx2,-hy2));
        *res++ = trafo.apply(this->center() + PointF64( hx2, hy2));
        *res++ = trafo.apply(this->center() + PointF64(-hx2, hy2));
    }
    //@}

    /***********************************/
    //! @name geometrische Manipulationen
    //@{
    virtual Rect & translate(PointF64 const & v);
    virtual Rect & rotate(Angle const & alpha);
    //! Erzeugt ein neues um @a v verschobenes Rechteck
    Rect const getTranslate(PointF64 const & v) const;
    //! Erzeugt ein neues um @a alpha gedrehtes Rechteck
    Rect const getRotate(Angle const & alpha) const;
    //@}

    /***********************************/
    //! @name debug ausgabe
    //@{
    virtual std::ostream & print(std::ostream & os) const;
    virtual bool validate() const;
    //@}

private:

    //! Größe des Rechteckes
    /*! @sa Rect::size
     */
    PointF64 size_;

    //! Winkel zur x-Achse
    Angle alpha_;

    /*! @name Serialisierung
     * @sa @ref serialization
     */
    //@{
    friend class boost::serialization::access;
    template<typename Archive>
    void save(Archive & ar, UN32 /*version*/) const {
        ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(SShape)
           << BOOST_SERIALIZATION_NVP(size_)
           << BOOST_SERIALIZATION_NVP(alpha_);
    }
    template<typename Archive>
    void load(Archive & ar, UN32 /*version*/) {
        using boost::serialization::make_nvp;
        Rect tmp(PointF64(0,0), PointF64(0,0));
        ar >> make_nvp(BOOST_PP_STRINGIZE(SShape),
                       boost::serialization::base_object<SShape>(tmp))
           >> make_nvp(BOOST_PP_STRINGIZE(size_), tmp.size_)
           >> make_nvp(BOOST_PP_STRINGIZE(alpha_), tmp.alpha_);
        *this = tmp;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER();
    //@}
};

IPL_NS_END

#endif
