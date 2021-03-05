add_executable(test_rrc_event test_rrc_event.c)
target_link_libraries(test_rrc_event fb_rrc_event_msgs)
add_dependencies(test test_rrc_event)

add_executable(e2e_rrc_event e2e_rrc_event.c)
target_link_libraries(e2e_rrc_event
                      PUBLIC
                        fb_rrc_event_rf
                        fb_rrc_event_app
                        e2ap_server
                        pthread
                        flexric_agent)
add_dependencies(test e2e_rrc_event)
