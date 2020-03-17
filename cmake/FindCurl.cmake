# - Try to find curl

find_package(PkgConfig)

find_library(CURL_LIBRARY NAMES curl)

set(CURL_LIBRARY ${CURL_LIBRARY} CACHE PATH "Path to curl library")

include(FindPackageHandleStandardArgs)

mark_as_advanced(
    CURL_FOUND
    CURL_INCLUDE_DIRS
    CURL_LIBRARIES
    CURL_LIBRARY_DIRS
    CURL_FLAGS)
