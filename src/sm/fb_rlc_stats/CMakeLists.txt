add_library(fb_rlc_stats_msgs OBJECT rlc_stats_msg_enc.c rlc_stats_msg_dec.c)
target_link_libraries(fb_rlc_stats_msgs PUBLIC types e2sm_rlc_stats_fb_lib)
target_include_directories(fb_rlc_stats_msgs
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_rlc_stats>")

add_library(fb_rlc_stats_rf STATIC rlc_stats_rf.c)
target_link_libraries(fb_rlc_stats_rf PUBLIC fb_rlc_stats_msgs flexric_agent e2sm_rlc_stats_fb_lib m)
target_include_directories(fb_rlc_stats_rf
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_rlc_stats>")
install(FILES rlc_stats_defs.h rlc_stats_rf.h DESTINATION include/flexric/fb_rlc_stats)
