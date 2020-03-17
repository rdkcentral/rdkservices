# - Try to find RDK Storage Manager library
# Once done this will define
#  NOPOLL_FOUND - System has Nopoll
#  NOPOLL_LIBRARIES - The libraries needed to use Nopoll

find_package(PkgConfig)

find_path(NOPOLL_INCLUDE_DIRS NAMES nopoll.h PATH_SUFFIXES nopoll)
#find_path(NOPOLL_INCLUDE_DIRS NAMES libIARM.h PATH_SUFFIXES rdk/iarmbus)

find_library(NOPOLL_LIBRARIES NAMES nopoll)

mark_as_advanced(
    NOPOLL_FOUND
    NOPOLL_INCLUDE_DIRS
    NOPOLL_LIBRARIES)