/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2011
 *              all rights reserved
 *
 * $Id: config.hh.cmake 5915 2012-06-19 11:30:24Z hari $
 *
 * @brief  compile configuration (template)
 *
 ********************************************************************/

/*! @page configuration Compile Time Configuration
 *
 * The library  uses @c cmake  to configure and  generate the make-  or solution
 * file see
 *     http://www.cmake.org
 *
 * For detailed  build instructions see the  @b README in the  root directory of
 * this project.
 *
 * To edit the settings use @c ccmake or @c cmake-gui.
 * The most important settings are
 *
 * - @b IPL_ASSERT_ENABLED activates the assert modus, see @ref assertions.
 * - @b IPL_DEBUG_ENABLED  activates debug logging. The  corresponding log level
 *   must be enabled too, see @ref logging.
 * - @b  IPL_ENABLE_DOXYGEN This  enables  doxygen to  generate  the docu.  This
 *   option requires doxygen in the  path and generates two makefile targets, @c
 *   api_doc  and  @c internal_doc  to  generate  the  API-  and  the  developer
 *   documentation respectively.
 * - @b IPL_ENABLE_LOG4CPLUS Enables logging via @c log4cplus, see @ref logging.
 * - @b IPL_ENABLE_NLS activates the native language support, see @ref i18n.
 * - @b BUILD_TESTING  enables the test suite  if the @c  cppunit test framework
 *   can be detected, see @ref unit_tests.
 * - @b IPL_ENABLE_BENCHMARK activates the benchmark suite, see @ref benchmarks.
 * - @b IPL_NO_RANDOM_SEED if this option is active, the random number generator
 *   starts always  with the  same seed. This  option is very  convenient during
 *   debugging  of  randomized methods  like  @c LMS  or  @c  RANSAC. With  this
 *   options,  the results of  those algorithms  are reproducible.  Normally the
 *   seed for the random number generator at startup is set to the system time.
 * - @b  IPL_BOARD_CIMG   activates  the  simple  graphical  output  device  via
 *   <tt>CImg</tt>. When  activated, the library has  additional dependencies to
 *   X11 on UNIX or gdi on Windows.
 *
 * @cond developer_docu
 *
 * @section configfile The configuration file @c config.hh
 *
 * All  these  configurations are  written  during  the  configure step  to  the
 * configuration    file     @c    config.hh    in     the    build    directory
 * <tt>/build/iplplus/include/</tt>. Remember that this file will be overwritten
 * in  each   configure  step,  do  not   modify  it,  but   the  template  file
 * <tt>config.hh.cmake</tt> in the  source tree, if you add  a new configuration
 * variable.
 *
 * The file @c config.hh must be  included before any other header in the source
 * file, even before the system includes, because it contains compiler dependent
 * macros.
 *
 * @endcond
 */

#ifndef IPL_CONFIG_HH
#define IPL_CONFIG_HH

/// @{
//! Defines für den ipl-namespace
#define IPL_NS_BEGIN namespace ipl {
#define IPL_NS_END }
/// @}

/// @{
//! Defines für einen anonymen namespace
#define IPL_ANON_NS_BEGIN namespace {
#define IPL_ANON_NS_END }
/// @}

//! Maschine, auf der diese iplplus compiliert wurde
#define IPL_SITE_NAME "${IPL_SITE_NAME}"

//! Verwendeter c++ Compiler
#define IPL_CXX_NAME "${CMAKE_CXX_COMPILER}"

//! Verwendete c++ compile flags
#define IPL_CXX_FLAGS "${IPL_CXX_FLAGS}"

//! Name der in gettext verwendeten domain
#define IPL_GETTEXT_DOMAIN "ipl"

/*! gesetzt, falls der Zufallsgenerator mit einem festen Startwert initialisiert
    werden soll
 */
#cmakedefine IPL_NO_RANDOM_SEED

/*! gesetzt wenn opencv version < 2.3 ist
*/
#cmakedefine IPL_USE_OLD_CV

/*! gesetzt, falls wir auf einem bigendian system sind
*/
#cmakedefine IPL_BIG_ENDIAN
#if !defined(IPL_BIG_ENDIAN)
     //! gesetzt, falls wir auf einem little endian system sind
#    define IPL_LITTLE_ENDIAN
#endif

/*! gesetzt, falls das Logging-Framework @c log4cplus vorhanden ist
 */
#cmakedefine IPL_HAVE_LOG4CPLUS

/*! gesetzt, falls NativeLanguageSupport über gettext aktiviert ist
 */
#cmakedefine IPL_ENABLE_NLS

/*! gesetzt, falls für die Serialisierung png-Support über die libpng
    vorhanden ist
 */
#cmakedefine IPL_HAVE_PNG

/*! gesetzt, falls für die Serialisierung jpeg-Support über die libjpg
    vorhanden ist
 */
#cmakedefine IPL_HAVE_JPEG

/*! gesetzt, falls für die Serialisierung tiff-Support über die
    libtiff vorhanden ist
 */
#cmakedefine IPL_HAVE_TIFF

//! welche grafikausgaben sind aktiviert?
//@{
#cmakedefine IPL_BOARD_CIMG
//@}

//! Interner ipl header
#define IPL_INTERNAL_USE

//! Funktionsname für das Logging
#if defined(__GNUC__)
#    define IPL_FNC_NAME __FUNCTION__
#elif defined(_MSC_VER)
#    define IPL_FNC_NAME __FUNCTION__
#else
#    define IPL_FNC_NAME __func__
#endif

//! keine Macros min und max
/*! windos.h definiert ansonsten wirklich Makros min resp. max, und dann
    compilert natürlich nichts mehr
 */
#define NOMINMAX

//! Kompatibilität für verschiedene Funktionen
#if defined(__GNUC__)

#elif defined(_MSC_VER)
#   define snprintf(buf, cnt, fmt, ...) _snprintf(buf, cnt, fmt, __VA_ARGS__)
#endif

#if defined(_MSC_VER)
#    include <ciso646>
     //'int': Variable wird auf booleschen Wert gesetzt
#    pragma warning (disable : 4800)
     //Konvertierung von 'ipl::N64' in 'ipl::F64', möglicher Datenverlust
#    pragma warning (disable : 4244)
     // Meldung: "You have used a std:: construct that is not safe.
#    pragma warning (disable : 4996)
#endif

#endif //IPL_CONFIG_HH

/// Local Variables:
/// mode: c++
/// End:
