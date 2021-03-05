option(REST_NORTHBOUND "Enable RESTful northbound API for the management of FlexRAN" OFF)
if(REST_NORTHBOUND)
  add_definitions(-DREST_NORTHBOUND)
endif()
MESSAGE(STATUS "REST_NORTHBOUND ${REST_NORTHBOUND}")

find_package(Boost 1.54.0 REQUIRED COMPONENTS program_options)

add_subdirectory(app)
if(REST_NORTHBOUND)
  add_subdirectory(north_api)
endif()
add_subdirectory(rib)

add_executable(flexran_rtc flexran_rtc.cc)
target_link_libraries(flexran_rtc
                      PUBLIC
                        Boost::program_options
                        e2ap_server
                        flexran_apps
                        flexran_rib)
if(REST_NORTHBOUND)
  target_link_libraries(flexran_rtc PUBLIC flexran_northbound)
endif()
target_include_directories(flexran_rtc PUBLIC flexran_apps)
target_link_libraries(flexran_rtc
                      PUBLIC
                        fb_rrc_event_app)
