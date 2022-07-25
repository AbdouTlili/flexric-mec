#############################
# E2 Agent test  
#############################


#############################
# E2 Agent 1  
#############################

add_executable(agent_1 
              test_agent.c
              ../../../test/sm/common/fill_ind_data.c)

target_link_libraries(agent_1
                      PUBLIC 
                      e2_agent
                      ${FlatCC} 
                      -pthread
                      -lsctp
                      -ldl)

target_compile_definitions(agent_1 PRIVATE TEST_AGENT_1)

#############################
# E2 Agent 2  
#############################

add_executable(agent_2 
              test_agent.c
              ../../../test/sm/common/fill_ind_data.c)

target_link_libraries(agent_2
                      PUBLIC 
                      e2_agent
                      ${FlatCC} 
                      -pthread
                      -lsctp
                      -ldl)

target_compile_definitions(agent_2 PRIVATE TEST_AGENT_2)

#############################
# E2 Agent 3  
#############################

add_executable(agent_3 
              test_agent.c
              ../../../test/sm/common/fill_ind_data.c)

target_link_libraries(agent_3
                      PUBLIC 
                      e2_agent
                      ${FlatCC} 
                      -pthread
                      -lsctp
                      -ldl)

target_compile_definitions(agent_3 PRIVATE TEST_AGENT_3)

#############################
# E2 Agent 4  
#############################

add_executable(agent_4 
              test_agent.c
              ../../../test/sm/common/fill_ind_data.c)

target_link_libraries(agent_4
                      PUBLIC 
                      e2_agent
                      ${FlatCC} 
                      -pthread
                      -lsctp
                      -ldl)

target_compile_definitions(agent_4 PRIVATE TEST_AGENT_4)

#############################
# E2 Agent 46  
#############################

add_executable(agent_5 
              test_agent.c
              ../../../test/sm/common/fill_ind_data.c)

target_link_libraries(agent_5
                      PUBLIC 
                      e2_agent
                      ${FlatCC} 
                      -pthread
                      -lsctp
                      -ldl)

target_compile_definitions(agent_5 PRIVATE TEST_AGENT_5)



