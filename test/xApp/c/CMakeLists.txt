
add_executable(test_xapp 
                test.c
                ../../../src/sm/mac_sm/ie/mac_data_ie.c
                ../../../src/sm/rlc_sm/ie/rlc_data_ie.c
                ../../../src/sm/pdcp_sm/ie/pdcp_data_ie.c
                ../../../src/sm/slice_sm/ie/slice_data_ie.c
                ../../../src/util/alg_ds/alg/defer.c
                )

target_link_libraries(test_xapp
                      PUBLIC
                      e42_xapp
                      -pthread
                      -lsctp
                      -ldl
                      )

