add_library(flexran_apps
            stats_manager.cc
            rslicing_manager.cc)
target_include_directories(flexran_apps PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(flexran_apps PUBLIC e2ap_server flexran_rib)
# the used SMs:
target_link_libraries(flexran_apps
                      PUBLIC
                        fb_mac_stats_msgs
                        fb_rslicing_msgs
                        fb_rrc_event_app)
