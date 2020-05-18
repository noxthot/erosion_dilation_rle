/*****************************************************************//**
 *
 * \file
 * \author fesc
 * \date   Apr 14 2008
 * \par    Copyright: &copy; Datacon 2008
 *              all rights reserved
 *
 * $Id: iplerr.cc 16 2012-09-05 13:08:00Z fesc $
 *
 * \brief  Implementation for all ipl error classes
 *
 ********************************************************************/

#include "ipl/iplerr.hh"

#include <cstdio>

IPL_NS_BEGIN

/*** IplError ***/
IplError::IplError(std::string const & msg)
    : msg_(msg)
{}

IplError::~IplError() throw()
{}

const char*
IplError::what() const throw()
{
    return msg_.c_str();
}

/*** ParameterError ***/
ParameterError::ParameterError(N32 nr,
                               std::string const & msg)
    : IplError(msg)
{
    char sz[100];
    snprintf(sz, sizeof(sz), ": invalid parameter #%d", nr);
    msg_ += sz;
}

ParameterError::~ParameterError() throw()
{}

/*** NotImplementedYetError ***/
NotImplementedYetError::NotImplementedYetError(std::string const & msg)
    : IplError(msg)
{}

NotImplementedYetError::~NotImplementedYetError() throw()
{}

/*** IoError ***/
IoError::IoError(std::string const & msg)
    : IplError(msg)
{}

IoError::~IoError() throw()
{}

/*** EmptyImgError ***/
EmptyImgError::EmptyImgError(std::string const & msg)
    : IplError(msg)
{}

EmptyImgError::~EmptyImgError() throw()
{}

/*** EmptyRegionError ***/
EmptyRegionError::EmptyRegionError(std::string const & msg)
    : IplError(msg)
{}

EmptyRegionError::~EmptyRegionError() throw()
{}

ProcessingError::ProcessingError(std::string const & msg)
    : IplError(msg)
{}

ProcessingError::~ProcessingError() throw()
{}

IPL_NS_END
