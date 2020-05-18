/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2011
 *              all rights reserved
 *
 * $Id: rbo.cc 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Implementation for ipl::Rbo
 *
 ********************************************************************/

#include "ipl/rbo.hh"

using namespace std;

IPL_NS_BEGIN

Rbo &
Rbo::translate(PointN16 const & v)
{
    start_ += v;
    return *this;
}

Rbo const
Rbo::getTranslate(PointN16 const & v) const
{
    Rbo r(*this);
    return r.translate(v);
}

std::ostream &
Rbo::print(std::ostream & os) const
{
    return os << this->start() << " len: " << this->len();
}

IPL_NS_END
