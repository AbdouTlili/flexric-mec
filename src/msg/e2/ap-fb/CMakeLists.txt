add_library(e2ap_fb_lib INTERFACE)
target_include_directories(e2ap_fb_lib
                           INTERFACE
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           "$<INSTALL_INTERFACE:include>")
target_link_libraries(e2ap_fb_lib INTERFACE FlatCC::FlatCC)
