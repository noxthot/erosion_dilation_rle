/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: log.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Logging macros
 *
 ********************************************************************/

/*! @page logging Logging Framework
 *
 * If the build system detects the logging framework @c log4cplus, we can enable
 * and use it, otherwise we write simply to @c clog.
 *
 * Log  messages  at  the  ipl::LogLevel  DEBUG  are  activated  during  compile
 * configuration and needs that the symbol @c IPL_DEBUG_ENABLED is defined.
 *
 * @section Initialization
 *
 * The  log system  must be  initialized in  the application  before  calling an
 * <tt>IPLplus</tt> function. If  we use only a simple logger,  this step can be
 * skipped.
 *
 * @c log4cplus allows a detailed  configuration, different output devices and a
 * hierarchy of loggers for different classes. To configure a simple logger that
 * writes to the console, we need only two lines of code
 * @code
 * log4cplus::BasicConfigurator::doConfigure();
 * iplLogger.setLogLevel(log4cplus::WARN_LOG_LEVEL);
 * @endcode
 *
 * A complex application  needs a more complex logging, writes  to a file, needs
 * time stamps in  the logs and maybe a thread-id. To achieve this, we configure
 * the logger via a file and load this file in the initialization.
 * @code
 * log4cplus::PropertyConfigurator::doConfigure(cfgfile);
 * @endcode
 * A sample  configuration file  @b log.cfg can  be found in  the top-directory.
 * There is also a mini-application @c iplinfo, that shows all active loggers of
 * the library.
 *
 * @section logging_viewers Logfile Viewer
 *
 * If the XML-Socket logger is active, the  best way to view the log file is the
 * chainsaw utility, see
 *
 * http://logging.apache.org/chainsaw/index.html
 *
 * To activate this appender, use the  @c SOCK Appender in the log configuration
 * file.
 *
 * @cond developer_docu
 * @section logging_macros Usage of the Logging Macros
 *
 * All  log   messages  are  written   trough  the  macros   #IPLLOG_DEBUG  till
 * #IPLLOG_FATAL, a  direct usage of the  logger object must be  avoided. To log
 * complex messages use the stream operator of the logstream
 * @code
 * IPLLOG_INFO("entering " << IPL_FNC_NAME << ": Binaries the image "
 *             << img << " with threshold " << thresh);
 * @endcode
 *
 * If ones would use the @c ipl Logger in an application, import the root logger
 * via an @c using declaration from <tt>ipl namespace</tt>
 * @code
 * using ipl::iplLogger;
 * @endcode
 * But the  normal way is to instantiate  your own logger from  @c log4cplus and
 * use it.
 *
 * @par Time Measurement
 * To  measure  time  spans @c  IPLplus  has  a  special  logger and  the  macro
 * IPLLOG_TIMINGS, that can be used in the following way
 * @code
 * PictImage img;
 * Timer tm;
 * tm.tic();
 * img.binarize(0, 128);
 * IPLLOG_TIMING("binarize", tm.toc());
 * @endcode
 *
 * @section class_logger Loggers for New Classes
 *
 * If you add a  new class to the library, provide a  logger for this class. For
 * that     use     the     two     macros     #IPL_DECLARE_CLASS_LOGGER     and
 * #IPL_DEFINE_CLASS_LOGGER to declare/define such a new logger.
 *
 * Declare the logger in the class header
 * @code
 * protected:
 * IPL_DECLARE_CLASS_LOGGER;
 * @endcode
 * and define it in the implementation file
 * @code
 * IPL_DEFINE_CLASS_LOGGER(PictImage, ipl.pictimage)
 * @endcode

 * The first parameter of the second  macro is exactly the class name, for which
 * to define  the new logger, the second  is the name with  its parent. Remember
 * that @c log4cpus has  a hierarchy of loggers and we use  the library root for
 * the parent. In cases with subclassing  you may use a second hierarchy like
 * @c ipl.base.derived
 *
 * Don't forget to update the configuration file log.cfg.
 *
 * @endcond
 */

#ifndef IPL_LOG_HH
#define IPL_LOG_HH

#include "ipl/config.hh"
#include "ipl/ipltypes.hh"
#include "ipl/loglevel.hh"

#ifdef IPL_HAVE_LOG4CPLUS
#   include <log4cplus/logger.h>
#   include <log4cplus/configurator.h>
#else
#   include <iostream>
#endif

IPL_NS_BEGIN

/// Macros zum Loggen auf den verschiedenen Levels
/*! Es sollten einzig und allein diese Macros verwendet werden, und
  nicht direkt mit dem Logger gearbeitet werden.
  @sa @ref logging
 */
/// @{
#ifdef IPL_HAVE_LOG4CPLUS
#   ifdef IPL_DEBUG_ENABLED
#       define IPLLOG_DEBUG(B) LOG4CPLUS_DEBUG(iplLogger, B)
#   else
#       define IPLLOG_DEBUG(B)
#   endif
#   define IPLLOG_INFO(B)     LOG4CPLUS_INFO(iplLogger, B)
#   define IPLLOG_WARN(B)     LOG4CPLUS_WARN(iplLogger, B)
#   define IPLLOG_ERROR(B)    LOG4CPLUS_ERROR(iplLogger, B)
#   define IPLLOG_FATAL(B)    LOG4CPLUS_FATAL(iplLogger, B)
#   define IPLLOG_TIMING(H,T) LOG4CPLUS_INFO(timeLogger, H << " took " << T << " musec");
#else
#   ifdef IPL_DEBUG_ENABLED
#       define IPLLOG_DEBUG(B) {\
                std::clog << "DEBUG: " << B << "\n"; }
#       define IPLLOG_INFO(B) {\
                std::clog << "INFO: " << B << "\n"; }
#       define IPLLOG_TIMING(H,T) {\
                std::clog << H << " took " << T << " musec" << "\n"; }
#   else
#       define IPLLOG_DEBUG(B)
#       define IPLLOG_INFO(B)
#       define IPLLOG_TIMING(H,T)
#   endif
#   define IPLLOG_WARN(B) {\
                std::clog << "WARN: " << B << "\n"; }
#   define IPLLOG_ERROR(B) {\
                std::clog << "ERROR: " << B << "\n"; }
#   define IPLLOG_FATAL(B) {\
                std::clog << "FATAL: " << B << "\n"; }
#endif
/// @}

//! Setzt eine Property des Loggers
/*! @a (key,value) ist ein Wertepaar wie es auch das Konfigurationsfile enthält
 */
void logSetProperty(std::string const & key,
                    std::string const & value);

/*! Das globale Loggingobjekt, oder der RootLogger, falls log4cplus
 vorhanden ist
*/
//@{
#ifdef IPL_HAVE_LOG4CPLUS
    extern log4cplus::Logger iplLogger;
    extern log4cplus::Logger timeLogger;
#endif
//@}

//! Macros zum Erzeugen eines Class-Loggers
/*! @cond developer_docu
 * Siehe auch @ref class_logger
 * @endcond
 */
/// @{
#ifdef IPL_HAVE_LOG4CPLUS
#    define IPL_DECLARE_CLASS_LOGGER static log4cplus::Logger iplLogger
#else
#    define IPL_DECLARE_CLASS_LOGGER static int dummy_nothing
#endif

#ifdef IPL_HAVE_LOG4CPLUS
#    define IPL_DEFINE_CLASS_LOGGER(CLASS, NAME) \
            log4cplus::Logger CLASS::iplLogger = \
            log4cplus::Logger::getInstance(#NAME)
#else
#    define IPL_DEFINE_CLASS_LOGGER(CLASS, NAME) int CLASS::dummy_nothing = 0
#endif
/// @}

IPL_NS_END

#endif
