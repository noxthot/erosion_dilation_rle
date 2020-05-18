/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: type_manip.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief Sammelstelle für Typenmanipulationen
 *
 * Innhalt wurde zum Großteil aus loki/TypeManip.h gestohlen
 *
 ********************************************************************/

#ifndef IPL_TYPE_MANIP_HH
#define IPL_TYPE_MANIP_HH

#include "ipl/config.hh"

#include <type_traits>

#include <ipl/ipltypes.hh>

IPL_NS_BEGIN

///@cond developer_docu

//! Check, if @a T is a native ipltype
template<typename T>
struct IsIplType :
    public std::integral_constant<bool,
         std::is_same<T, N8>::value
         or std::is_same<T, UN8>::value
         or std::is_same<T, N16>::value
         or std::is_same<T, UN16>::value
         or std::is_same<T, N32>::value
         or std::is_same<T, UN32>::value
         or std::is_same<T, N64>::value
         or std::is_same<T, UN64>::value
         or std::is_same<T, F64>::value>
{};
///@endcond

IPL_NS_END

#endif
