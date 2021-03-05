add_library(res_virt OBJECT res_virt.cc nvs.cc)
target_include_directories(res_virt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(recursive_slicing
               recursive_slicing.cc
               conn_manager.cc
               mac_stats_manager.cc
               rslicing_manager.cc)
target_link_libraries(recursive_slicing
                      PUBLIC
                        e2ap_server
                        flexric_agent
                        res_virt
                        fb_mac_stats_msgs
                        fb_rslicing_msgs
                        fb_rrc_event_app
                        fb_rrc_conf_msgs
                        pthread)
