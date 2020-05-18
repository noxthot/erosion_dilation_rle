/*****************************************************************//**
 *
 * \file
 * \author fesc
 * \date   Apr 14 2008
 * \par    Copyright: &copy; Datacon 2008
 *              all rights reserved
 *
 * $Id: loglevel.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * \brief  the loglevels
 *
 ********************************************************************/

#ifndef IPL_LOGLEVEL_HH
#define IPL_LOGLEVEL_HH

#include "ipl/config.hh"

IPL_NS_BEGIN

// Workarround, manche dieser Konstanten werden von windows gesetzt
#if defined(DEBUG)
#undef DEBUG
#endif
#if defined(INFO)
#undef INFO
#endif
#if defined(WARN)
#undef WARN
#endif
#if defined(ERROR)
#undef ERROR
#endif
#if defined(FATAL)
#undef FATAL
#endif

//! Die verschiedenen Loglevels
class LogLevel {
public:

    //! Die verschiedenen Loglevels
    enum Level {
        DEBUG = 10000,
        INFO  = 20000,
        WARN  = 30000,
        ERROR = 40000,
        FATAL = 50000
    };
};

IPL_NS_END

#endif
