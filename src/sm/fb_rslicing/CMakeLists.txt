add_library(fb_rslicing_msgs OBJECT rslicing_msg_enc.c rslicing_msg_dec.c)
target_link_libraries(fb_rslicing_msgs PUBLIC types e2sm_rslicing_fb_lib)
target_include_directories(fb_rslicing_msgs
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_rslicing>")

add_library(fb_rslicing_rf STATIC rslicing_rf.c)
target_link_libraries(fb_rslicing_rf PUBLIC fb_rslicing_msgs flexric_agent e2sm_rslicing_fb_lib)
target_include_directories(fb_rslicing_rf
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include/flexric/fb_rslicing>")
install(FILES rslicing_defs.h rslicing_rf.h DESTINATION include/flexric/fb_rslicing)
