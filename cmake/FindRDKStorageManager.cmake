# - Try to find RDK Storage Manager library
# Once done this will define
#  RDK_SM_FOUND - System has DS
#  RDK_SM_LIBRARIES - The libraries needed to use DS

find_package(PkgConfig)

find_library(RDK_SM_LIBRARIES NAMES rdkstmgr)

mark_as_advanced(
    RDK_SM_FOUND
    RDK_SM_LIBRARIES)