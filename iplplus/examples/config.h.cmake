/*****************************************************************//**
 *
 * \file
 * \author fesc
 * \date   Apr 14 2008
 * \par    Copyright: &copy; Datacon 2008
 *              all rights reserved
 *
 * $Id: config.h.cmake 1582 2009-06-17 08:21:45Z fesc $
 *
 * \brief  Template für die compile Konfiguration
 *
 ********************************************************************/

#ifndef CONFIG_H
#define CONIFG_H

#define cimg_debug ${CIMG_ENABLE_DEBUG}

#define IMAGE_PATH "${PROJECT_SOURCE_DIR}/images/"

#if defined(_MSC_VER) && _MSC_VER >= 1300
#   define _CRT_SECURE_NO_DEPRECATE 1
#endif

#endif

/// Local Variables:
/// mode: c++
/// End:
