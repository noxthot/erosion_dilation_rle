/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008
 *              all rights reserved
 *
 * $Id: logger.cc 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Implementation for ipl::Logger
 *
 ********************************************************************/

#include "ipl/log.hh"

#include <iostream>

#ifdef IPL_HAVE_LOG4CPLUS
#   include <log4cplus/logger.h>
#   include <log4cplus/helpers/property.h>
#   include <log4cplus/configurator.h>
#endif

IPL_NS_BEGIN

#ifdef IPL_HAVE_LOG4CPLUS
log4cplus::Logger iplLogger = log4cplus::Logger::getInstance("ipl");
log4cplus::Logger timeLogger = log4cplus::Logger::getInstance("ipl.timelogger");
void logSetProperty(std::string const & key,
                    std::string const & value)
{
    log4cplus::helpers::Properties props;
    props.setProperty(key, value);
    log4cplus::PropertyConfigurator cf(props);
    cf.configure();
}
#else
void logSetProperty(std::string const & /*key*/,
                    std::string const & /*value*/)
{
}
#endif

IPL_NS_END
