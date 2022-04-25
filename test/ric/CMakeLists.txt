#############################
# NEAR-RIC using nng 
#############################

#add_executable(near_ric_nng near_ric_nng.c )
#target_link_libraries(near_ric_nng
#                     PUBLIC 
#                      near_ric
#                      ${FlatCC} 
                      #ToDO: get rid of nng dependency
                      #                      -lnng
                      #-pthread
                      #-lsctp
                      #-ldl)

###########
# NEAR-RIC with iApp 
###########

add_executable(near_ric_iapp near_ric_iapp.c )
target_link_libraries(near_ric_iapp
                     PUBLIC 
                      near_ric
                      ${FlatCC} 
                      #ToDO: get rid of nng dependency
                      #-lnng
                      -pthread
                      -lsctp
                      -ldl)



