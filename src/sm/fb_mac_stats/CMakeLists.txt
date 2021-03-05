add_library(fb_mac_stats_msgs OBJECT mac_stats_msg_enc.c mac_stats_msg_dec.c)
target_link_libraries(fb_mac_stats_msgs PUBLIC types e2sm_mac_stats_fb_lib)
target_include_directories(fb_mac_stats_msgs
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_mac_stats>")

add_library(fb_mac_stats_rf STATIC mac_stats_rf.c)
target_link_libraries(fb_mac_stats_rf PUBLIC fb_mac_stats_msgs flexric_agent e2sm_mac_stats_fb_lib)
target_include_directories(fb_mac_stats_rf
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_mac_stats>")
install(FILES mac_stats_defs.h mac_stats_rf.h DESTINATION include/flexric/fb_mac_stats)
