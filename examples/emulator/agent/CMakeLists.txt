#############################
# E2 Agent test  
#############################


#############################
# E2 Agent 1  
#############################

add_executable(emu_agent1
                  test_agent.c
                  ../../../test/sm/common/fill_ind_data.c)

target_link_libraries(emu_agent1
                      PUBLIC 
                      e2_agent
                      ${FlatCC} 
                      -pthread
                      -lsctp
                      -ldl)

#############################
# E2 Agent 2
#############################

add_executable(emu_agent2
                test_agent.c
                ../../../test/sm/common/fill_ind_data.c)

target_link_libraries(emu_agent2
                        PUBLIC
                        e2_agent
                        ${FlatCC}
                        -pthread
                        -lsctp
                        -ldl)

#############################
# E2 Agent 3
#############################

add_executable(emu_agent3
                test_agent.c
                ../../../test/sm/common/fill_ind_data.c)

target_link_libraries(emu_agent3
                        PUBLIC
                        e2_agent
                        ${FlatCC}
                        -pthread
                        -lsctp
                        -ldl)
