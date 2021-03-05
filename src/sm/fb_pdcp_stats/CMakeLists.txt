add_library(fb_pdcp_stats_msgs OBJECT pdcp_stats_msg_enc.c pdcp_stats_msg_dec.c)
target_link_libraries(fb_pdcp_stats_msgs PUBLIC types e2sm_pdcp_stats_fb_lib)
target_include_directories(fb_pdcp_stats_msgs
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_pdcp_stats>")

add_library(fb_pdcp_stats_rf STATIC pdcp_stats_rf.c)
target_link_libraries(fb_pdcp_stats_rf PUBLIC fb_pdcp_stats_msgs flexric_agent e2sm_pdcp_stats_fb_lib m)
target_include_directories(fb_pdcp_stats_rf
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_pdcp_stats>")
install(FILES pdcp_stats_defs.h pdcp_stats_rf.h DESTINATION include/flexric/fb_pdcp_stats)
