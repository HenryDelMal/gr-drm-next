# Locate libopus.
#
# Opus_INCLUDE_DIR - directory containing opus/opus.h
# Opus_LIBRARIES   - Opus library
# Opus_FOUND       - whether both were found

find_path(Opus_INCLUDE_DIR opus/opus.h HINTS ENV OPUS_ROOT PATH_SUFFIXES include)
find_library(Opus_LIBRARY NAMES opus HINTS ENV OPUS_ROOT PATH_SUFFIXES lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Opus REQUIRED_VARS Opus_LIBRARY Opus_INCLUDE_DIR)
set(Opus_LIBRARIES ${Opus_LIBRARY})
mark_as_advanced(Opus_LIBRARY Opus_INCLUDE_DIR)
