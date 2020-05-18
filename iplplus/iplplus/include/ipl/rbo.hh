/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2011
 *              all rights reserved
 *
 * $Id: rbo.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Header for ipl::Rbo
 *
 ********************************************************************/

#ifndef IPL_RBO_HH
#define IPL_RBO_HH

#include "ipl/config.hh"

#include <iostream>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "ipl/ipltypes.hh"
#include "ipl/point.hh"

IPL_NS_BEGIN

class Region;

//! Ein Rbo (row-blob), dh. eine waagerechte Zeile im Bild
/*! Ein Rbo ist ein Teil einer Bildzeile. Er wird repräsentiert
 * durch den Startpunkt und die Länge der Linie.
 */
class Rbo
{
public:

    Rbo()
        : start_(PointN16(0,0)),
          len_(1)
    {}
    //! ctr
    Rbo(PointN16 const & start,
        N32 len)
        : start_(start),
          len_(len)
    {}

    // autogenerierte Methoden sind ok
    //Rbo(Rbo const & other);
    //Rbo & operator=(Rbo const & rhs);

    //! dtr
    ~Rbo()
    {}

    //! Liefert den Startpunkt
    PointN16 const & start() const {
        return start_;
    }

    //! Liefert die Länge
    N32 len() const {
        return len_;
    }

    //! Verschiebt den Rbo um den Vektor @a v
    /*! @sa getTranslate
     */
    Rbo & translate(PointN16 const & v);

    //! Liefert einen neuen, um @a v verschobenen, Rbo
    /*! @sa translate
     */
    Rbo const getTranslate(PointN16 const & v) const;

    //! debug ausgabe
    std::ostream & print(std::ostream & os) const;
private:

    //! Startpunkt des Rbo
    PointN16 start_;

    //! Länge des Rbo
    N32 len_;

    friend class Region;

    /*! @name Serialisierung
     * @sa @ref serialization
     */
    //@{
    friend class boost::serialization::access;
    template<typename Archive>
    void save(Archive & ar, UN32 /*version*/) const {
        ar << BOOST_SERIALIZATION_NVP(start_)
           << BOOST_SERIALIZATION_NVP(len_);
    }
    template<typename Archive>
    void load(Archive & ar, UN32 /*version*/) {
        using boost::serialization::make_nvp;
        Rbo tmp;
        ar >> make_nvp(BOOST_PP_STRINGIZE(start_), tmp.start_)
           >> make_nvp(BOOST_PP_STRINGIZE(len_), tmp.len_);
        *this = tmp;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER();
    //@}
};

//! Vergleich
/*! Dies ist die induzierte lexikographische Ordnung. Es wird aber nur der
 * Startpunkt verglichen
 * @relates Rbo
*/
inline bool
operator<(Rbo const & lhs, Rbo const & rhs) {
    return lhs.start() < rhs.start();
}

//! Gleichheit
/*!
  @relates Rbo
*/
inline bool
operator==(Rbo const & lhs, Rbo const & rhs) {
    return lhs.start() == rhs.start()
        && lhs.len() == rhs.len();
}

//! Streamausgabe
/*! @relates Rbo
 */
inline
std::ostream &
operator<<(std::ostream & os, Rbo const & r) {
    return r.print(os);
}

IPL_NS_END

#endif
