add_library(e2sm_pdcp_stats_fb_lib INTERFACE)
target_include_directories(e2sm_pdcp_stats_fb_lib
                           INTERFACE
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           "$<INSTALL_INTERFACE:include>")
target_link_libraries(e2sm_pdcp_stats_fb_lib INTERFACE FlatCC::FlatCC)
install(FILES
          flatbuffers_common_reader.h
          pdcp_stats_reader.h
          flatbuffers_common_builder.h
          pdcp_stats_builder.h
        DESTINATION
          include/flexric/fb_pdcp_stats)
