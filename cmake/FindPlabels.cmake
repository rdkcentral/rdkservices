find_package(PkgConfig)

find_path(PLABELS_INCLUDE_DIRS NAMES pbnj_utils.hpp PATH_SUFFIXES pbnj_utils)

find_library(PLABELS_LIBRARIES NAMES plabels)

set(PLABELS_FLAGS -DUSE_PLABELS=1 -DRDKLOG_ERROR= -DRDKLOG_INFO= CACHE PATH "Flags for pbnj_utils")

mark_as_advanced(
        PLABELS_FLAGS
        PLABELS_INCLUDE_DIRS
        PLABELS_LIBRARIES)
