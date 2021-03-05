add_library(relay_app_rf OBJECT relay_app_rf.c)
target_link_libraries(relay_app_rf
                      PUBLIC
                        types
                        flexric_agent
                        e2ap_server
                        e2ap_ds_lib
                        pthread)
target_include_directories(relay_app_rf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
