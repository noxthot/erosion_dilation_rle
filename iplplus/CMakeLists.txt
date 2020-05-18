#**************************************************************
#
# @file
# @author fesc
# @date   Apr 14 2008
# @par    Copyright: &copy; Datacon 2008-2012
#              all rights reserved
#
# $Id: CMakeLists.txt 17 2012-09-05 13:22:45Z fesc $
#
# @brief  Hauptkonfigurationsfile für die lib iplplus
#
# Hier werden die meisten Konfigurationen gesetzt, außerdem wird eine Liste der
# zusätzlich benötigten Libraries IPL_LIBRARIES erstellt, gegen die dann das
# eigentlich Target iplplus linken muss
#
#**************************************************************

project(iplplus)

# cmake-Umgebung aufsetzen
cmake_minimum_required(VERSION 2.8.3)
set(css_SOURCE_DIR ${PROJECT_SOURCE_DIR}/..)
set(css_BINARY_DIR ${PROJECT_BINARY_DIR}/..)
set(CMAKE_MODULE_PATH ${css_SOURCE_DIR}/CMakeModules)

# makrosammlung
include(CSS-Macros)

# use standard installation paths, this allows debian multiarch
include(GNUInstallDirs)

# versionsnummer der library
# MAJOR.MINOR wird als ABI-Version verwendet (SOVERSION in cmake)
set(IPL_VERSION_MAJOR 0)
set(IPL_VERSION_MINOR 30)
set(IPL_VERSION_PATCH 0)
svn_revision(${PROJECT_SOURCE_DIR} IPL)
css_set_experimental_option()
css_compose_version_string(IPL ${CSS_EXPERIMENTAL_BUILD})

# Outputdirectory für alle binaries
css_output_dir(IPL)

#stelle den Buildtype defaultmäßig auf Debug
css_set_buildtype("Release")

# shared libs funktionieren so nur unter Linux
if(NOT BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS
    "if on, build iplplus as shared library"
    ON
    )
endif()
mark_as_advanced(BUILD_SHARED_LIBS)

### compiler spezifische dinger
# gcc-warnings
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -W -Wall")
  #-Wold-style-cast -Woverloaded-virtual -Weffc++
endif()

# Liste mit den zusätzlich benötigten LIBS
set(IPL_LIBRARIES)

# Debugoptionen sind bei BUILD_TYPE Debug defaultmäßig an, ansonsten aus
# Releaseoptionen umgekehrt
if(CSS_BUILD_TYPE STREQUAL "Debug")
  set(DEBUG_DEFAULT ON)
  set(RELEASE_DEFAULT OFF)
else()   #d.h. Release, RelWithDebInfo oder MinSizeRel
  set(DEBUG_DEFAULT OFF)
  set(RELEASE_DEFAULT ON)
endif()

option(IPL_ASSERT_ENABLED
  "enable ipl specific assertions"
  ${DEBUG_DEFAULT}
  )
if(IPL_ASSERT_ENABLED)
  add_definitions(-DIPL_ASSERT_ENABLED)
endif()

option(IPL_DEBUG_ENABLED
  "enable ipl debug messages"
  ${DEBUG_DEFAULT}
  )
if(IPL_DEBUG_ENABLED)
  add_definitions(-DIPL_DEBUG_ENABLED)
endif()

option(IPL_NO_RANDOM_SEED
  "disable the seed of the random number generator and start allways with the same sequence (useful for debugging monte carlo methods)"
  ${DEBUG_DEFAULT}
  )
mark_as_advanced(IPL_NO_RANDOM_SEED)

include(TestBigEndian)
test_big_endian(IPL_BIG_ENDIAN)

### Obligatorische Libraries
#set(Boost_DEBUG TRUE)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost 1.40 REQUIRED serialization)
include_directories(${Boost_INCLUDE_DIRS})
list(APPEND IPL_LIBRARIES ${Boost_LIBRARIES})
# die unten brauche ich nur optional
find_package(Boost 1.40 COMPONENTS program_options regex)

find_package(CImg 1.3.3 REQUIRED)
include_directories(${CIMG_INCLUDE_DIR})

### Optionale Libraries
# check ob log4cplus vorhanden ist, wenn ja, wird es aktiviert und die
# entsprechende lib dazugenommen
find_package(LOG4CPLUS)
option(
  IPL_ENABLE_LOG4CPLUS
  "enable logging via log4cplus framework"
  ${LOG4CPLUS_FOUND}
  )
if(IPL_ENABLE_LOG4CPLUS)
  set(IPL_HAVE_LOG4CPLUS "YES")
  list(APPEND IPL_LIBRARIES ${LOG4CPLUS_LIBRARIES})
  include_directories(${LOG4CPLUS_INCLUDE_DIR})
  if(NOT EXISTS ${PROJECT_BINARY_DIR}/log.cfg)
    configure_file(
      ${PROJECT_SOURCE_DIR}/log.cfg
      ${PROJECT_BINARY_DIR}/log.cfg
      COPYONLY
      )
  endif()
else()
  set(IPL_HAVE_LOG4CPLUS "NO")
endif()

# checke ob doxygen vorhanden ist, wenn ja, dann die API-Doku aktivieren
find_package(Doxygen)
option(
  IPL_ENABLE_DOXYGEN
  "enable doxygen for building the api-doc"
  ${DOXYGEN_FOUND}
  )

# falls vorhanden, verwendet die IPL für die Serialisierung von Bildern diese
# libs, falls nicht, kann sie diese Formate halt nicht lesen
set(IPL_HAVE_PNG false)
find_package(PNG)
option(IPL_USE_PNG
  "enable support for png image format"
  ${PNG_FOUND}
  )
if(IPL_USE_PNG)
  set(IPL_HAVE_PNG true)
  list(APPEND IPL_LIBRARIES ${PNG_LIBRARIES})
  include_directories(${PNG_INCLUDE_DIR})
endif()
mark_as_advanced(IPL_USE_PNG)

set(IPL_HAVE_JPEG false)
find_package(JPEG)
option(IPL_USE_JPEG
  "enable support for jpeg image format"
  ${JPEG_FOUND}
  )
if(IPL_USE_JPEG)
  set(IPL_HAVE_JPEG true)
  list(APPEND IPL_LIBRARIES ${JPEG_LIBRARIES})
  include_directories(${JPEG_INCLUDE_DIR})
endif()
mark_as_advanced(IPL_USE_JPEG)

set(IPL_HAVE_TIFF false)
find_package(TIFF)
option(IPL_USE_TIFF
  "enable support for tiff image format"
  ${TIFF_FOUND}
  )
if(IPL_USE_TIFF)
  set(IPL_HAVE_TIFF true)
  list(APPEND IPL_LIBRARIES ${TIFF_LIBRARIES})
  include_directories(${TIFF_INCLUDE_DIR})
endif()
mark_as_advanced(
  IPL_USE_TIFF
  TIFF_LIBRARY
  TIFF_INCLUDE_DIR
  )

### Graphikausgaben
# falls gewählt, werden die entsprechenden Klassen mitcompiliert und eine
# Grafikausgabe mit Hilfe dieser Klassen steht dann zur Verfügung

# Display via CImg
find_package(CImg)
if(CIMG_FOUND AND CIMG_LIBRARIES)
  set(_tmp ON)
else()
  set(_tmp OFF)
endif()
option(IPL_BOARD_CIMG
  "use the CImg-Library for displaying"
  ${_tmp}
  )
if(IPL_BOARD_CIMG)
  include_directories(${CIMG_INCLUDE_DIR})
  list(APPEND IPL_LIBRARIES ${CIMG_LIBRARIES})
endif()

#########################################
# Opencv liefert ein cmake-configfile mit, ich muss es nur finden
# zu den Suchpfaden vgl die docu zu find_package
# FIXME: OpenCV behandelt die Komponenten nicht richtig, linkt aber die
# libraries ohne vollen Pfad, daher trage ich die Komponenten einfach händisch
# ein. Außerdem heißen die libraries je nach Version verschieden
find_package(OpenCV REQUIRED)
include_directories(BEFORE ${OpenCV_INCLUDE_DIRS})
if(OpenCV_VERSION VERSION_LESS "2.3")
 list(APPEND IPL_LIBRARIES "cxcore;cv")
 set(IPL_USE_OLD_CV TRUE)
else()
  list(APPEND IPL_LIBRARIES "opencv_core;opencv_imgproc;opencv_calib3d")
  set(IPL_USE_OLD_CV FALSE)
endif()

### nun ist alles aufgesetzt für den Compile der lib
add_subdirectory(iplplus)

### Beispiele dazu generieren
add_subdirectory(examples)

### unit testing
# checke ob cppunit installiert ist und aktiviere gegebenenfalls die tests
find_package(CppUnit)
set(CTEST_MEMORYCHECK_COMMAND valgrind)
set(DART_TESTING_TIMEOUT 240 CACHE STRING
  "Maximum time allowed before CTest will kill the test.")
option(BUILD_TESTING "Build the testing tree." ${CPPUNIT_FOUND})
include(CTest)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()

include(FeatureSummary)
feature_summary(WHAT ALL)
