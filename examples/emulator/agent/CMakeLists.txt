#############################
# E2 Agent test  
#############################


#############################
# E2 Agent gNB
#############################

add_executable(emu_agent_gnb
                  test_agent.c
                  ../../../test/sm/common/fill_ind_data.c)

target_link_libraries(emu_agent_gnb
                      PUBLIC
                      e2_agent
                      ${FlatCC} 
                      -pthread
                      -lsctp
                      -ldl)

target_compile_definitions(emu_agent_gnb PRIVATE TEST_AGENT_GNB)

#############################
# E2 Agent gNB-CU
#############################

add_executable(emu_agent_gnb_cu
                test_agent.c
                ../../../test/sm/common/fill_ind_data.c)

target_link_libraries(emu_agent_gnb_cu
                        PUBLIC
                        e2_agent
                        ${FlatCC}
                        -pthread
                        -lsctp
                        -ldl)

target_compile_definitions(emu_agent_gnb_cu PRIVATE TEST_AGENT_GNB_CU)

#############################
# E2 Agent gNB-DU
#############################

add_executable(emu_agent_gnb_du
                test_agent.c
                ../../../test/sm/common/fill_ind_data.c)

target_link_libraries(emu_agent_gnb_du
                        PUBLIC
                        e2_agent
                        ${FlatCC}
                        -pthread
                        -lsctp
                        -ldl)

target_compile_definitions(emu_agent_gnb_du PRIVATE TEST_AGENT_GNB_DU)

#############################
# E2 Agent eNB
#############################

add_executable(emu_agent_enb
                test_agent.c
                ../../../test/sm/common/fill_ind_data.c)

target_link_libraries(emu_agent_enb
                        PUBLIC
                        e2_agent
                        ${FlatCC}
                        -pthread
                        -lsctp
                        -ldl)

target_compile_definitions(emu_agent_enb PRIVATE TEST_AGENT_ENB)
