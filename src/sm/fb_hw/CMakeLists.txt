add_library(e2sm_hw_fb_rf e2sm_hw_fb_rf.c)
target_link_libraries(e2sm_hw_fb_rf PUBLIC types flexric_agent e2sm_hw_fb_lib)
target_include_directories(e2sm_hw_fb_rf
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include>")

add_library(e2sm_hw_fb_app e2sm_hw_fb_app.c)
target_link_libraries(e2sm_hw_fb_app PUBLIC types e2ap_server e2sm_hw_fb_lib -lpthread)
target_include_directories(e2sm_hw_fb_app
                           PUBLIC
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                             "$<INSTALL_INTERFACE:include>")
