# Locate a DRM-enabled FAAD2 without bundling it in this project. Current
# FAAD2 releases build DRM support as a separate faad_drm library, while many
# distributions expose DRM declarations in the header even when the ordinary
# faad library does not provide NeAACDecInitDRM.
find_library(Faad_LIBRARY NAMES faad_drm faad faad2 HINTS ENV FAAD_ROOT PATH_SUFFIXES lib)
if(Faad_LIBRARY)
  get_filename_component(_Faad_REAL_LIBRARY "${Faad_LIBRARY}" REALPATH)
  get_filename_component(_Faad_LIBDIR "${_Faad_REAL_LIBRARY}" DIRECTORY)
  get_filename_component(_Faad_PREFIX "${_Faad_LIBDIR}" DIRECTORY)
  find_path(Faad_INCLUDE_DIR neaacdec.h HINTS "${_Faad_PREFIX}/include" NO_DEFAULT_PATH)
endif()
find_path(Faad_INCLUDE_DIR neaacdec.h HINTS ENV FAAD_ROOT PATH_SUFFIXES include)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Faad REQUIRED_VARS Faad_LIBRARY Faad_INCLUDE_DIR)

if(Faad_FOUND)
  include(CheckLibraryExists)
  check_library_exists("${Faad_LIBRARY}" NeAACDecInitDRM "" Faad_HAS_DRM)
  if(NOT Faad_HAS_DRM)
    message(FATAL_ERROR
      "The selected FAAD2 library (${Faad_LIBRARY}) was built without DRM "
      "decoder support. Install libfaad_drm and set FAAD_ROOT to its prefix.")
  endif()
endif()

set(Faad_LIBRARIES ${Faad_LIBRARY})
mark_as_advanced(Faad_LIBRARY Faad_INCLUDE_DIR Faad_HAS_DRM)
