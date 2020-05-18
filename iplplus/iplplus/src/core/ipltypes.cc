/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Feb 16, 2011
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: ipltypes.cc 16 2012-09-05 13:08:00Z fesc $
 *
 * @brief  Implementiert die Funktionen aus ipltypes.hh
 *
 ********************************************************************/

#include "ipl/ipltypes.hh"

#include "ipl/validable.hh"

#define _(A) (A)

IPL_NS_BEGIN

char const *
ObjectColor::toString(Color c) {
    switch (c) {
    case Dark:
        return _("dark");
    case Bright:
        return _("bright");
    case Arbitrary:
        return _("arbitrary");
    }
    return "unknown";
}

char const *
AccuracyMode::toString(Mode m) {
    switch (m) {
    case Normal:
        return _("normal");
    case Fast:
        return _("fast");
    case Accurate:
        return _("accurate");
    }
    return _("unknown");
}

IPL_NS_END
