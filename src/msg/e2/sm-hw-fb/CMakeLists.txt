add_library(e2sm_hw_fb_lib INTERFACE)
target_include_directories(e2sm_hw_fb_lib
                           INTERFACE
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           "$<INSTALL_INTERFACE:include>")
target_link_libraries(e2sm_hw_fb_lib INTERFACE FlatCC::FlatCC)
