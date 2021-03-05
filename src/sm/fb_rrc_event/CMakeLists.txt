add_library(fb_rrc_event_msgs OBJECT rrc_event_msg_enc.c rrc_event_msg_dec.c)
target_link_libraries(fb_rrc_event_msgs PUBLIC types e2sm_rrc_fb_lib)
target_include_directories(fb_rrc_event_msgs
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_rrc>")

add_library(fb_rrc_event_app STATIC rrc_event_app.c)
target_link_libraries(fb_rrc_event_app
                      PUBLIC
                        fb_rrc_event_msgs
                        e2ap_server
                        e2sm_rrc_fb_lib
                        e2ap_ds_lib)
target_include_directories(fb_rrc_event_app
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_rrc>")

add_library(fb_rrc_event_rf STATIC rrc_event_rf.c)
target_link_libraries(fb_rrc_event_rf PUBLIC fb_rrc_event_msgs flexric_agent e2sm_rrc_fb_lib)
target_include_directories(fb_rrc_event_rf
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_rrc>")
install(FILES rrc_event_defs.h rrc_event_rf.h DESTINATION include/flexric/fb_rrc)
