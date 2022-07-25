#############################
# E2E Test E2-AGENT NEAR-RIC 
#############################

add_executable(test_near_ric 
              test_near_ric.c 
              ../sm/common/fill_ind_data.c)

target_link_libraries(test_near_ric
                      PUBLIC 
                      e2_agent
                      ${FlatCC} 
                      #/home/mir/workspace/flatcc/lib/libflatccrt_d.a
                      near_ric_test
                      #ToDO: get rid of nng dependency
                      #-lnng
                      -pthread
                      -lsctp
                      -ldl)


