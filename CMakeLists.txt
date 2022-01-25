cmake_minimum_required(VERSION 3.15)

project(flexric VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

### Require out-of-source builds
#file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
#if(EXISTS "${LOC_PATH}")
#    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
#endif()

set(default_build_type "Debug")

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

set(COMPILER "GCC" CACHE STRING "Compiler selection")
set_property(CACHE COMPILER PROPERTY STRINGS "GCC" "CLANG")

if(COMPILER STREQUAL CLANG)
  #  add_compile_options("$<$<CONFIG:DEBUG>:-fblocks;>")
  #  add_link_options("$<$<CONFIG:DEBUG>:-lBlocksRuntime;>")
  add_compile_options("-fblocks;")
  add_link_options("-lBlocksRuntime;")
endif()

set(BUILDING_LIBRARY "STATIC" CACHE STRING "Static or dynamic library")
set_property(CACHE BUILDING_LIBRARY PROPERTY STRINGS "STATIC" "DYNAMIC")
message(STATUS "Selected LIBRARY TYPE: ${BUILDING_LIBRARY}")


set(SANITIZER "NONE" CACHE STRING "Sanitizers")
set_property(CACHE SANITIZER PROPERTY STRINGS "NONE" "ADDRESS" "THREAD")
message(STATUS "Selected SANITIZER TYPE: ${SANITIZER}")

if(SANITIZER STREQUAL "ADDRESS")
   add_compile_options("$<$<CONFIG:DEBUG>:-fno-omit-frame-pointer;-fsanitize=address>")
   add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
elseif(SANITIZER STREQUAL "THREAD")
  add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=thread;-g;>")
  add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=thread;>")
endif()

option(CODE_COVERAGE "Code coverage" OFF)
if(CODE_COVERAGE)
  add_compile_options("$<$<CONFIG:DEBUG>:-fprofile-arcs;-ftest-coverage>")
  add_link_options("$<$<CONFIG:DEBUG>:-lgcov;-coverage;>")
endif()

option(CODE_PROFILER "Code Profiler" OFF)
if( CODE_PROFILER )
  add_compile_options("$<$<CONFIG:DEBUG>:-pg>")
endif()

option(INCL_WHAT_YOU_USE "Include what you use tool: iwyu" OFF)



if(BUILDING_LIBRARY STREQUAL "STATIC")
  add_compile_options("$<$<CONFIG:DEBUG>:-W;-Wall;-Wextra;-g>")
elseif(BUILDING_LIBRARY STREQUAL "DYNAMIC")
  # -fPIC flag
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  add_compile_options("$<$<CONFIG:DEBUG>:-W;-Wall;-Wextra;-g>")
else()
  message(FATAL_ERROR "Unknown building type. Either choose a static or a dynamic library")

endif()


set(E2AP_ENCODING "ASN" CACHE STRING "The E2AP encoding to use")
set_property(CACHE E2AP_ENCODING PROPERTY STRINGS "ASN" "FLATBUFFERS")
message(STATUS "Selected E2AP_ENCODING: ${E2AP_ENCODING}")


#######
## Service Models 
#######

# MAC Service Model
set(SM_ENCODING_MAC "PLAIN" CACHE STRING "The MAC SM encoding to use")
set_property(CACHE SM_ENCODING_MAC PROPERTY STRINGS "PLAIN" "ASN" "FLATBUFFERS")
message(STATUS "Selected MAC SM_ENCODING: ${SM_ENCODING_MAC}")

# RLC Service Model
set(SM_ENCODING_RLC "PLAIN" CACHE STRING "The RLC SM encoding to use")
set_property(CACHE SM_ENCODING_RLC PROPERTY STRINGS "PLAIN" "ASN" "FLATBUFFERS")
message(STATUS "Selected RLC SM_ENCODING: ${SM_ENCODING_RLC}")

# PDCP Service Model
set(SM_ENCODING_PDCP "PLAIN" CACHE STRING "The PDCP SM encoding to use")
set_property(CACHE SM_ENCODING_PDCP PROPERTY STRINGS "PLAIN" "ASN" "FLATBUFFERS")
message(STATUS "Selected PDCP SM_ENCODING: ${SM_ENCODING_PDCP}")




########
### Flatbuffer 
########
set(FlatCC_INCLUDE_DIR "" CACHE STRING "The Flatbuffers include directory")
set(FlatCC_LIB_DIR "" CACHE STRING "The Flatbuffers lib directory")

if(E2AP_ENCODING STREQUAL "FLATBUFFERS")
  find_library(FlatCC
    NAMES flatccrt_d
    HINTS ${FlatCC_LIB_DIR} 
    #"~/workspace/flatcc/lib/"
    )
endif()



#find_package(FlatCC)
#if((E2AP_ENCODING STREQUAL "FLATBUFFERS") AND (NOT FlatCC_FOUND))
#  message(FATAL_ERROR "E2AP_ENCODING set to FLATBUFFERS, but FlatCC NOT FOUND")
#endif()


#find_package(Git QUIET)
#if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
## Update submodules as needed
#  option(GIT_SUBMODULE "Check submodules during build" OFF)
#    if(GIT_SUBMODULE)
#        message(STATUS "Updating git submodules")
#        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
#                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
#        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
#            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
#        endif()
#    endif()
#endif()

include_directories(src)
add_subdirectory(src)
add_subdirectory(test)


SET(CMAKE_EXPORT_COMPILE_COMMANDS ON )
# For YouCompleteMe configuration
IF( EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" )
  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
  )
ENDIF()



###
# Install the Service models
###
install(TARGETS mac_sm DESTINATION /usr/lib/flexric)
install(TARGETS rlc_sm DESTINATION /usr/lib/flexric)
install(TARGETS pdcp_sm DESTINATION /usr/lib/flexric)
install(TARGETS slice_sm DESTINATION /usr/lib/flexric)

###
# Install the configuration file
###

install(FILES flexric.conf DESTINATION /usr/lib/flexric)

#install(TARGETS flexric_agent DESTINATION /usr/lib/flexric)
#install(TARGETS near_ric DESTINATION /usr/lib/flexric)


if(FALSE)

install(TARGETS
          flexric_agent
          #          e2ap_ap
          # e2ap_ep
          # e2ap_ds_lib
        EXPORT flexric_agent_targets
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include/flexric
)

if(E2AP_ENCODING STREQUAL "ASN")
install(TARGETS
          e2ap_asn1_lib
        EXPORT flexric_agent_targets
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include/flexric
)
elseif(E2AP_ENCODING STREQUAL "FLATBUFFERS")
install(TARGETS
          e2ap_fb_lib
        EXPORT flexric_agent_targets
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include/flexric
)
endif()

if(FlatCC_FOUND)
install(TARGETS
          e2sm_mac_stats_fb_lib
          fb_mac_stats_msgs
          fb_mac_stats_rf
          e2sm_rlc_stats_fb_lib
          fb_rlc_stats_msgs
          fb_rlc_stats_rf
          e2sm_pdcp_stats_fb_lib
          fb_pdcp_stats_msgs
          fb_pdcp_stats_rf
          e2sm_rrc_fb_lib
          fb_rrc_stats_msgs
          fb_rrc_stats_rf
          fb_rrc_conf_msgs
          fb_rrc_conf_rf
        EXPORT flexric_agent_targets
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include/flexric
)
endif()

install(EXPORT flexric_agent_targets
        FILE FlexricAgentTargets.cmake
        NAMESPACE Flexric::
        DESTINATION lib/cmake/FlexricAgent
)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/FlexricAgentConfig.cmake"
  INSTALL_DESTINATION lib/cmake/FlexricAgent
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/FlexricAgentConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/FlexricAgentConfigVersion.cmake"
        DESTINATION lib/cmake/FlexricAgent
)

set(version 0.2.1)

set_property(TARGET flexric_agent PROPERTY VERSION ${version})
set_property(TARGET flexric_agent PROPERTY SOVERSION 0)
set_property(TARGET flexric_agent PROPERTY
  INTERFACE_FlexricAgent_MAJOR_VERSION 0)
set_property(TARGET flexric_agent APPEND PROPERTY
  COMPATIBLE_INTERFACE_STRING FlexricAgent_MAJOR_VERSION
)

# generate the version file for the config file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/FlexricAgentConfigVersion.cmake"
  VERSION "${version}"
  COMPATIBILITY ExactVersion
)

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Robert Schmidt <robert.schmidt@eurecom.fr>")
set(CPACK_PACKAGE_VENDOR "EURECOM")
set(CPACK_PACKAGE_DESCRIPTION "A library to integrate an E2 north-bound interface towards the O-RAN RIC.")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
include(CPack)

endif()
