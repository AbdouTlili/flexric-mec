add_executable(stats_test_agent stats_test_agent.c)
target_link_libraries(stats_test_agent
                      PUBLIC
                        fb_mac_stats_rf
                        fb_rslicing_rf
                        fb_pdcp_stats_rf
                        fb_rlc_stats_rf
                        fb_rrc_stats_rf
                        fb_rrc_conf_rf
                        fb_rrc_event_rf
                        flexric_agent
                        pthread)
