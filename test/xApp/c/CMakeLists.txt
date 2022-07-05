
add_executable(test_xapp 
                test.c
                # This sucks, profoundly
                ../../../src/sm/mac_sm/ie/mac_data_ie.c
                ../../../src/sm/rlc_sm/ie/rlc_data_ie.c
                ../../../src/sm/pdcp_sm/ie/pdcp_data_ie.c
                ../../../src/sm/slice_sm/ie/slice_data_ie.c
                ../../../src/sm/tc_sm/ie/tc_data_ie.c
                ../../../src/util/alg_ds/alg/defer.c
                )

target_link_libraries(test_xapp
                      PUBLIC
                      e42_xapp
                      -pthread
                      -lsctp
                      -ldl
                      )

add_executable(xapp_mac_rlc_pdcp_moni
                xapp_mac_rlc_pdcp_moni.c
                ../../../src/sm/mac_sm/ie/mac_data_ie.c
                ../../../src/sm/rlc_sm/ie/rlc_data_ie.c
                ../../../src/sm/pdcp_sm/ie/pdcp_data_ie.c
                ../../../src/sm/slice_sm/ie/slice_data_ie.c
                ../../../src/sm/tc_sm/ie/tc_data_ie.c
                ../../../src/util/alg_ds/alg/defer.c
                )

target_link_libraries(xapp_mac_rlc_pdcp_moni
                        PUBLIC
                        e42_xapp
                        -pthread
                        -lsctp
                        -ldl
                        )

add_executable(xapp_slice_moni_ctrl
                xapp_slice_moni_ctrl.c
                ../../../src/sm/mac_sm/ie/mac_data_ie.c
                ../../../src/sm/rlc_sm/ie/rlc_data_ie.c
                ../../../src/sm/pdcp_sm/ie/pdcp_data_ie.c
                ../../../src/sm/slice_sm/ie/slice_data_ie.c
                ../../../src/sm/tc_sm/ie/tc_data_ie.c
                ../../../src/util/alg_ds/alg/defer.c
                )

target_link_libraries(xapp_slice_moni_ctrl
                        PUBLIC
                        e42_xapp
                        -pthread
                        -lsctp
                        -ldl
                        )
