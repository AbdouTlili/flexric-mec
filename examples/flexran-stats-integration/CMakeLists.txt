cmake_minimum_required(VERSION 3.15)
project(StandaloneTestAgent)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_C_STANDARD 11)

set(FlatCC_HINT_INCLUDE_DIR "~/flatcc/include/")
set(FlatCC_LIBRARY_DIR "~/flatcc/lib/")
find_package(FlatCC REQUIRED)
find_package(FlexricAgent 0.2.0 REQUIRED)

add_executable(mac_test_agent mac_test_agent.c)
target_link_libraries(mac_test_agent
                      PRIVATE
                        Flexric::flexric_agent
                        Flexric::fb_mac_stats_rf)

add_executable(pdcp_test_agent pdcp_test_agent.c)
target_link_libraries(pdcp_test_agent
                      PRIVATE
                        Flexric::flexric_agent
                        Flexric::fb_pdcp_stats_rf)
add_executable(rrc_test_agent rrc_test_agent.c)
target_link_libraries(rrc_test_agent
                      PRIVATE
                        Flexric::flexric_agent
                        Flexric::fb_rrc_stats_rf
                        Flexric::fb_rrc_conf_rf
                        pthread)
