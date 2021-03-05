add_library(e2sm_rrc_fb_lib INTERFACE)
target_include_directories(e2sm_rrc_fb_lib
                           INTERFACE
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           "$<INSTALL_INTERFACE:include>")
target_link_libraries(e2sm_rrc_fb_lib INTERFACE FlatCC::FlatCC)
install(FILES
          flatbuffers_common_reader.h
          rrc_common_reader.h
          rrc_stats_reader.h
          rrc_conf_reader.h
          rrc_event_reader.h
          flatbuffers_common_builder.h
          rrc_common_builder.h
          rrc_stats_builder.h
          rrc_conf_builder.h
          rrc_event_builder.h
        DESTINATION
          include/flexric/fb_rrc)
