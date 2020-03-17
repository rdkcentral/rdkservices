# - Try to find ControlManager
# Once done this will define
#  CTRLM_FOUND - System has ControlManager
#  CTRLM_INCLUDE_DIRS - The ControlManager include directories
#

find_package(PkgConfig)

find_path(CTRLM_INCLUDE_DIRS NAMES ctrlm_ipc.h)

set(CTRLM_INCLUDE_DIRS ${CTRLM_INCLUDE_DIRS})
set(CTRLM_INCLUDE_DIRS ${CTRLM_INCLUDE_DIRS} CACHE PATH "Path to ControlManager include")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CTRLM DEFAULT_MSG CTRLM_INCLUDE_DIRS)

mark_as_advanced(
    CTRLM_FOUND
    CTRLM_INCLUDE_DIRS)
