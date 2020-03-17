# - Try to find Audio Capture Mgr
# Once done this will define
#  AC_FOUND - System has audiocapturemgr
#  AC_INCLUDE_DIRS - The audiocapturemgr include directories
#  AC_LIBRARIES - The libraries needed to use audiocapturemgr
#  AC_FLAGS - The flags needed to use audiocapturemgr
#

find_package(PkgConfig)

find_library(AC_LIBRARIES NAMES audiocapturemgr)
find_path(AC_INCLUDE_DIRS NAMES audiocapturemgr_iarm.h PATH_SUFFIXES audiocapturemgr)

set(AC_LIBRARIES ${AC_LIBRARIES} CACHE PATH "Path to audiocapturemgr library")
set(AC_INCLUDE_DIRS ${AC_INCLUDE_DIRS} )
set(AC_INCLUDE_DIRS ${AC_INCLUDE_DIRS} CACHE PATH "Path to audiocapturemgr include")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(AC DEFAULT_MSG AC_INCLUDE_DIRS AC_LIBRARIES)

mark_as_advanced(
    AC_FOUND
    AC_INCLUDE_DIRS
    AC_LIBRARIES
    AC_LIBRARY_DIRS
    AC_FLAGS)
