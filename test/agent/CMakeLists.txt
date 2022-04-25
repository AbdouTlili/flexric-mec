#############################
# E2 Agent test  
#############################


#############################
# E2 Agent 42  
#############################

add_executable(agent_42 
              test_agent.c
              ../common/fill_ind_data.c)

target_link_libraries(agent_42
                      PUBLIC 
                      e2_agent
                      ${FlatCC} 
                      -pthread
                      -lsctp
                      -ldl)

target_compile_definitions(agent_42 PRIVATE TEST_AGENT_42)

#############################
# E2 Agent 43  
#############################

add_executable(agent_43 
              test_agent.c
              ../common/fill_ind_data.c)

target_link_libraries(agent_43
                      PUBLIC 
                      e2_agent
                      ${FlatCC} 
                      -pthread
                      -lsctp
                      -ldl)

target_compile_definitions(agent_43 PRIVATE TEST_AGENT_43)

#############################
# E2 Agent 44  
#############################

add_executable(agent_44 
              test_agent.c
              ../common/fill_ind_data.c)

target_link_libraries(agent_44
                      PUBLIC 
                      e2_agent
                      ${FlatCC} 
                      -pthread
                      -lsctp
                      -ldl)

target_compile_definitions(agent_44 PRIVATE TEST_AGENT_44)

#############################
# E2 Agent 45  
#############################

add_executable(agent_45 
              test_agent.c
              ../common/fill_ind_data.c)

target_link_libraries(agent_45
                      PUBLIC 
                      e2_agent
                      ${FlatCC} 
                      -pthread
                      -lsctp
                      -ldl)

target_compile_definitions(agent_45 PRIVATE TEST_AGENT_45)

#############################
# E2 Agent 46  
#############################

add_executable(agent_46 
              test_agent.c
              ../common/fill_ind_data.c)

target_link_libraries(agent_46
                      PUBLIC 
                      e2_agent
                      ${FlatCC} 
                      -pthread
                      -lsctp
                      -ldl)

target_compile_definitions(agent_46 PRIVATE TEST_AGENT_43)

