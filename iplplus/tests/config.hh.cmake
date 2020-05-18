/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: config.hh.cmake 5942 2012-07-02 10:45:47Z fesc $
 *
 * @brief  Template für die compile Konfiguration
 *
 ********************************************************************/

#ifndef CONFIG_HH
#define CONFIG_HH

#include <cstdlib>
#include "ipl/config.hh"
#include "ipl/log.hh"

// bei Tests immer ohne displaying, sonst müsste ich ja
// gegen die richtige lib linken
#define cimg_display 0

#define IMAGE_PATH \
    getenv("IPL_TESTIMAGE_PATH") ? getenv("IPL_TESTIMAGE_PATH") : "${CMAKE_SOURCE_DIR}/images/"
#define TESTDATA_PATH \
    getenv("IPL_TESTDATA_PATH") ? getenv("IPL_TESTDATA_PATH") : "${CMAKE_CURRENT_SOURCE_DIR}/testdata/"
// Full path the logger-configfile used during test
#define TEST_LOGFILE \
    getenv("IPL_TEST_LOGFILE") ? getenv("IPL_TEST_LOGFILE") : "${CMAKE_SOURCE_DIR}/log.cfg"

#include <cstddef>
#include <cstring>
static bool localTesting()
{
    char const * lt = getenv("CSS_LOCAL_TESTING");
    return lt != 0 and strcmp(lt, "OFF") != 0;
}
#endif //CONFIG_HH

/// Local Variables:
/// mode: c++
/// End:
