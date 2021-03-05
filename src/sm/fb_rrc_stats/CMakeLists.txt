add_library(fb_rrc_stats_msgs OBJECT rrc_stats_msg_enc.c rrc_stats_msg_dec.c)
target_link_libraries(fb_rrc_stats_msgs PUBLIC types e2sm_rrc_fb_lib)
target_include_directories(fb_rrc_stats_msgs
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_rrc>")

add_library(fb_rrc_stats_rf STATIC rrc_stats_rf.c)
target_link_libraries(fb_rrc_stats_rf PUBLIC fb_rrc_stats_msgs flexric_agent e2sm_rrc_fb_lib)
target_include_directories(fb_rrc_stats_rf
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_rrc>")
install(FILES rrc_stats_defs.h rrc_stats_rf.h DESTINATION include/flexric/fb_rrc)
