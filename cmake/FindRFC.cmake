# - Try to find RFC components
# Once done this will define
#  RFC_FOUND - System has RFC
#  RFC_LIBRARIES - The libraries needed to use  RFC
#  RFC_INCLUDE_DIRS - The headers needed to use RFC

find_package(PkgConfig)

find_library(RFC_LIBRARIES NAMES rfcapi)
find_path(RFC_INCLUDE_DIRS NAMES rfcapi.h )
find_path(WDMP_INCLUDE_DIRS NAMES wdmp-c.h PATH_SUFFIXES wdmp-c)

set(RFC_INCLUDE_DIRS ${RFC_INCLUDE_DIRS} ${WDMP_INCLUDE_DIRS} )
set(RFC_INCLUDE_DIRS ${RFC_INCLUDE_DIRS} ${WDMP_INCLUDE_DIRS} CACHE PATH "Path to RFC include")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(RFC DEFAULT_MSG RFC_INCLUDE_DIRS RFC_LIBRARIES)

mark_as_advanced(
    RFC_FOUND
    RFC_INCLUDE_DIRS
    RFC_LIBRARIES)

