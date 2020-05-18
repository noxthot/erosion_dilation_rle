/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Jun 8, 2009
 * @par    Copyright: &copy; Datacon 2008-2009
 *              all rights reserved
 *
 * $Id: pict_instantiate.cc 14 2012-09-05 12:46:22Z fesc $
 *
 * @brief  Instanziiert die verwendeten Bildtypen explizit
 *
 * Die gesamte Implementierung muss in dieser Translationunit vorhanden sein,
 * daher werden alle pict_xxx.cc files hier includiert und nur dieses File
 * dem Buildsystem zum Compilieren übergeben
 *
 ********************************************************************/

#include "ipl/pict.hh"

#include "pict.cc"
#include "pict_segment.cc"
#include "pict_serialize.cc"
#include "pict_filter.cc"
#include "pict_render.cc"

IPL_NS_BEGIN

//! explizite Instanziierungen
//@{
template class PictImg<UN8>;
template class PictImg<N16>;
//@}


IPL_NS_END
