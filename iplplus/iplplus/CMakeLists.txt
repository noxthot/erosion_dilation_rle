#**************************************************************
#
# @file
# @author fesc
# @date   Apr 14 2008
# @par    Copyright: &copy; Datacon 2008-2012
#              all rights reserved
#
# $Id: CMakeLists.txt 2 2012-09-05 10:36:38Z fesc $
#
# @brief  Eigentlicher build der iplplus lib
#
#**************************************************************

# Liste mit den Source-Unterverzeichnissen; diese Liste muss erweitert werden,
# wenn weitere Verzeichisse dazukommen
set(SRC_DIRS
  core
  hal
  graphics
)

if(CMAKE_COMPILER_IS_GNUCXX)
  # extra Warnings für den eigentlichen IPL-Code
  set(IPL_FLAGS "-Wold-style-cast -Woverloaded-virtual")
  # -Weffc++")
  # Optionen für sse und konsorten
  if(NOT CSS_BUILD_TYPE STREQUAL Debug)
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES amd64* OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES x86_64*)
      option(IPL_USE_FAST_MATH "enable -ffast-math" ON)
      option(IPL_USE_MMX "enable mmx" ON)
      option(IPL_USE_SSE "enable sse" ON)
      option(IPL_USE_SSE2 "enable sse2" ON)
      option(IPL_USE_SSE3 "enable sse3" ON)
    elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES i686*)
      option(IPL_USE_FAST_MATH "enable -ffast-math" ON)
      option(IPL_USE_MMX "enable mmx" ON)
      option(IPL_USE_SSE "enable sse" ON)
      option(IPL_USE_SSE2 "enable sse2" OFF)
      option(IPL_USE_SSE3 "enable sse3" OFF)
    endif()
  endif()
endif()
if(IPL_USE_FAST_MATH)
  set(IPL_FLAGS "${IPL_FLAGS} -ffast-math")
endif()
if(IPL_USE_MMX)
  set(IPL_FLAGS "${IPL_FLAGS} -mmmx")
endif()
if(IPL_USE_SSE)
  set(IPL_FLAGS "${IPL_FLAGS} -msse")
endif()
if(IPL_USE_SSE2)
  set(IPL_FLAGS "${IPL_FLAGS} -msse2")
endif()
if(IPL_USE_SSE3)
  set(IPL_FLAGS "${IPL_FLAGS} -msse3")
endif()

site_name(IPL_SITE_NAME)

# Info zu den compile flags, hängen von der verwendeten Konfiguration ab
string(TOUPPER ${CSS_BUILD_TYPE} TMP)
set(IPL_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${TMP}} ${IPL_FLAGS}"
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/ipl/config.hh.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/include/ipl/config.hh
)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/ipl/version.hh.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/include/ipl/version.hh
)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

# Erstelle eine Liste aller Source-Files
foreach(d ${SRC_DIRS})
  include(src/${d}/CMakeLists.txt)
  FOREACH(s ${SOURCES})
    set(_srcFile src/${d}/${s})
    list(APPEND LIB_SRCS ${_srcFile})
  endforeach()
endforeach()

if(IPL_ENABLE_DOXYGEN)
  add_subdirectory(doc)
endif()

add_library(${CMAKE_PROJECT_NAME} ${LIB_SRCS})
target_link_libraries(${CMAKE_PROJECT_NAME} ${IPL_LIBRARIES} ${Boost_THREAD_LIBRARY})

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
  VERSION ${IPL_VERSION_MAJOR}.${IPL_VERSION_MINOR}.${IPL_VERSION_PATCH}
  SOVERSION ${IPL_VERSION_MAJOR}.${IPL_VERSION_MINOR}
  COMPILE_FLAGS ${IPL_FLAGS}
)

### Exportiere die Konfiguration, gilt aber nicht für den install-tree!
# TODO use WriteBasicConfigVersionFile with cmake 2.8.6
configure_file(
  ${PROJECT_SOURCE_DIR}/iplplusConfigVersion.cmake.in
  ${PROJECT_BINARY_DIR}/iplplusConfigVersion.cmake
  @ONLY
  )
configure_file(
  ${PROJECT_SOURCE_DIR}/iplplusConfig.cmake.in
  ${PROJECT_BINARY_DIR}/iplplusConfig.cmake
  @ONLY
  )
export(TARGETS iplplus FILE ${PROJECT_BINARY_DIR}/iplplus-exports.cmake)

# export der Konfiguration für den install-tree
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/iplplusInstallConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/iplplusConfig.cmake
  @ONLY
  )

# Installation
install(
  TARGETS ${CMAKE_PROJECT_NAME}
  EXPORT iplplus-exports
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
)
install(
  EXPORT iplplus-exports
  DESTINATION ${CMAKE_INSTALL_DATADIR}/iplplus     # adapt iplpusInstallConfig.cmake.in if you change this
  COMPONENT devel
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/iplplusConfig.cmake
        ${CMAKE_BINARY_DIR}/iplplusConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_DATADIR}/iplplus
  COMPONENT devel
)

install(
  DIRECTORY include/ipl
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT devel
  FILES_MATCHING PATTERN "*.hh"
  PATTERN ".svn" EXCLUDE
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/include/ipl/config.hh
        ${CMAKE_CURRENT_BINARY_DIR}/include/ipl/version.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ipl
  COMPONENT devel
)
