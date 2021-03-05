# FlatCC_FOUND - system has FlatCC
# Provides target FlatCC::FlatCC to link against FlatCC library

include(FindPackageHandleStandardArgs)

set(FlatCC_HINT_INCLUDE_DIR "" CACHE STRING "The Flatbuffers include directory")
set_property(CACHE FlatCC_HINT_INCLUDE_DIR PROPERTY TYPE STRING)

set(FlatCC_LIBRARY_DIR "" CACHE STRING "The Flatbuffers link directory")
set_property(CACHE FlatCC_LIBRARY_DIR PROPERTY TYPE STRING)

find_library(FlatCC_LIBRARY NAMES libflatccrt.a HINTS ${FlatCC_LIBRARY_DIR})
find_path(FlatCC_INCLUDE_DIR NAMES flatcc/flatcc.h HINTS ${FlatCC_HINT_INCLUDE_DIR})

find_package_handle_standard_args(FlatCC REQUIRED_VARS FlatCC_LIBRARY FlatCC_INCLUDE_DIR)

if (FlatCC_FOUND)
  mark_as_advanced(FlatCC_LIBRARY)
  mark_as_advanced(FlatCC_INCLUDE_DIR)
endif()

if (FlatCC_FOUND AND NOT TARGET FlatCC::FlatCC)
  add_library(FlatCC::FlatCC STATIC IMPORTED)
  set_property(TARGET FlatCC::FlatCC PROPERTY IMPORTED_LOCATION ${FlatCC_LIBRARY})
  target_include_directories(FlatCC::FlatCC INTERFACE ${FlatCC_INCLUDE_DIR})
endif()
