# - Try to find Log Milestome library
# Once done this will define
#  LM_LIBRARY - The library needed to use LogMilestone library
#

find_package(PkgConfig)

find_library(RDKL_LIBRARY NAMES rdkloggers)

set(RDKL_LIBRARY ${RDKL_LIBRARY} CACHE PATH "Path to rdkloggers library")

mark_as_advanced(RDKL_LIBRARY)


