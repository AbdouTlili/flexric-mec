
add_executable(xapp_slice
        xapp_slice.c
                ../../../../src/util/alg_ds/alg/defer.c
                ../../../../src/sm/mac_sm/ie/mac_data_ie.c
                ../../../../src/sm/rlc_sm/ie/rlc_data_ie.c
                ../../../../src/sm/pdcp_sm/ie/pdcp_data_ie.c
                ../../../../src/sm/slice_sm/ie/slice_data_ie.c
                ../../../../src/sm/tc_sm/ie/tc_data_ie.c
                )

target_link_libraries(xapp_slice
                      PUBLIC
                      e42_xapp
                      -pthread
                      -lsctp
                      -ldl
                      )



