add_library(fb_rrc_conf_msgs OBJECT rrc_conf_msg_enc.c rrc_conf_msg_dec.c)
target_link_libraries(fb_rrc_conf_msgs PUBLIC types e2sm_rrc_fb_lib)
target_include_directories(fb_rrc_conf_msgs
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_rrc>")

add_library(fb_rrc_conf_rf STATIC rrc_conf_rf.c)
target_link_libraries(fb_rrc_conf_rf PUBLIC fb_rrc_conf_msgs flexric_agent e2sm_rrc_fb_lib)
target_include_directories(fb_rrc_conf_rf
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_rrc>")
install(FILES rrc_conf_defs.h rrc_conf_rf.h DESTINATION include/flexric/fb_rrc)
