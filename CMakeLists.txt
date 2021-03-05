cmake_minimum_required(VERSION 3.15)

project(flexric-agent VERSION 0.2.2 LANGUAGES C CXX)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

set(default_build_type "Debug")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

add_compile_options("$<$<CONFIG:RELEASE>:-W;-Wall;-O2>")
add_compile_options("$<$<CONFIG:MINSIZEREL>:-W;-Wall;-O2>")
add_compile_options("$<$<CONFIG:RELWITHDEBINFO>:-W;-Wall;-O2;-g>")
add_compile_options("$<$<CONFIG:DEBUG>:-W;-Wall;-Wextra;-O0;-g>")
add_compile_options("$<$<CONFIG:DEBUG>:-pg;-fno-omit-frame-pointer;-fprofile-arcs;-ftest-coverage>")
add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
add_link_options("$<$<CONFIG:DEBUG>:-fno-omit-frame-pointer;-fsanitize=address;-lgcov;--coverage>")


SET( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set(E2AP_ENCODING "FLATBUFFERS" CACHE STRING "The E2AP encoding to use")
set_property(CACHE E2AP_ENCODING PROPERTY STRINGS "ASN" "FLATBUFFERS")
message(STATUS "Selected E2AP_ENCODING: ${E2AP_ENCODING}")
find_package(FlatCC)
if((E2AP_ENCODING STREQUAL "FLATBUFFERS") AND (NOT FlatCC_FOUND))
  message(FATAL_ERROR "E2AP_ENCODING set to FLATBUFFERS, but FlatCC NOT FOUND")
endif()

option(PROFILE_THROUGHPUT "Show profiling information for endpoint" OFF)
if(PROFILE_THROUGHPUT)
  add_definitions(-DPROFILE_THROUGHPUT)
endif()

option(SM_HW "Compile the HW iApp using ASN.1 definitions" OFF)

add_subdirectory(src)
add_subdirectory(test)


IF( EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" )
  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
  )
ENDIF()

install(TARGETS
          flexric_agent
          types
          e2ap_ep
          e2ap_ds_lib
          e2ap_msg_enc
          e2ap_msg_dec
          e2ap_encode
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
          e2sm_rslicing_fb_lib
          fb_rslicing_msgs
          fb_rslicing_rf
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
          fb_rrc_event_msgs
          fb_rrc_event_rf
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
set(version 0.2.2)

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
