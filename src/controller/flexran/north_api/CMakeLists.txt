find_package(PkgConfig REQUIRED)
pkg_check_modules(Pistache REQUIRED IMPORTED_TARGET libpistache)

add_library(flexran_northbound
            STATIC
              call_manager.cc
              rslicing_manager_calls.cc
              stats_manager_calls.cc)
target_include_directories(flexran_northbound PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# pistache, when installed from the PPA, needs SSL, crypto, pthread, so link it in
# also, Pistache::Pistache should be a valid target name, but then make
# complains with "No rule to make target # '/usr/lib/x86_64-linux-gnu/libpistache.so'"
# so we go with ${Pistache_LIBRARIES}
target_link_libraries(flexran_northbound PUBLIC flexran_apps ${Pistache_LIBRARIES} ssl crypto pthread)
