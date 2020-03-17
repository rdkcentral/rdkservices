# - Try to find Display Settings library
# Once done this will define
#  TTS_INCLUDE_DIRS - The TTS include directories
#  TTS_LIBRARIES - The libraries needed to use TTS
#

find_package(PkgConfig)
find_library(TTS_LIBRARIES NAMES TTSClient)
find_path(TTS_INCLUDE_DIRS NAMES TTSCommon.h)
find_path(TTSC_INCLUDE_DIRS NAMES TTSClient.h)

set(TTS_LIBRARIES ${TTS_LIBRARIES} CACHE PATH "Path to TTSClient")
set(TTS_INCLUDE_DIRS ${TTS_INCLUDE_DIRS} CACHE PATH "Path to TTS include")
set(TTSC_INCLUDE_DIRS ${TTSC_INCLUDE_DIRS} CACHE PATH "Path to TTSClient include")



include(FindPackageHandleStandardArgs)

mark_as_advanced(
    TTS_INCLUDE_DIRS
    TTSC_INCLUDE_DIRS
    TTS_LIBRARIES)


