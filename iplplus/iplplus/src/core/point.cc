/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2010
 *              all rights reserved
 *
 * $Id: point.cc 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Implementation for ipl::Point<T>
 *
 ********************************************************************/

#include "ipl/point.hh"
#include <boost/format.hpp>

IPL_NS_BEGIN



//! Spezialisierungen von Point<T>::as
//@{
// auf N16
template<> template<>
Point<N16> const
Point<N16>::as() const {
    return *this;
}
template<> template<>
Point<N32> const
Point<N16>::as() const {
    return Point<N32>(*this);
}
template<> template<>
Point<F64> const
Point<N16>::as() const {
    return Point<F64>(*this);
}

// auf N32
template<> template<>
Point<N16> const
Point<N32>::as() const {
    return Point<N16>(N16(this->x_), N16(this->y_));
}
template<> template<>
Point<N32> const
Point<N32>::as() const {
    return *this;
}
template<> template<>
Point<F64> const
Point<N32>::as() const {
    return Point<F64>(*this);
}

// auf F64
template<> template<>
Point<N16> const
Point<F64>::as() const {
    return Point<N16>(mathli::roundF(this->x_), mathli::roundF(this->y_));
}
template<> template<>
Point<N32> const
Point<F64>::as() const {
    return Point<N32>(mathli::roundF(this->x_), mathli::roundF(this->y_));
}
template<> template<>
Point<F64> const
Point<F64>::as() const {
    return *this;
}

//! Spezialisierungen von Point<T>::print
//@{
//! Spezialisierungen von Point<T>::print
template<>
std::ostream &
Point<N16>::print(std::ostream & os) const
{
    return os << "(" << this->x_ << "," << this->y_ << ")";
}
//! Spezialisierungen von Point<T>::print
template<>
std::ostream &
Point<N32>::print(std::ostream & os) const
{
    return os << "(" << this->x_ << "," << this->y_ << ")";
}
//! Spezialisierungen von Point<T>::print
template<>
std::ostream &
Point<F64>::print(std::ostream & os) const
{
    return os << boost::format("(%1$.3f,%2$.3f)") % this->x_ % this->y_;
}
//@}

//! explizite Instanziierungen
//@{
template class Point<N16>;
template class Point<N32>;
template class Point<F64>;
//@}

IPL_NS_END
