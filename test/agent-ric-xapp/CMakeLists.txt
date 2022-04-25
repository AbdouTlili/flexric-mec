if(E2AP_ENCODING STREQUAL "ASN")
add_executable(test_ag_ric_xapp 
                test_ag_ric_xapp.c
                ../common/fill_ind_data.c
                ../../src/sm/mac_sm/ie/mac_data_ie.c
                ../../src/sm/rlc_sm/ie/rlc_data_ie.c
                ../../src/sm/pdcp_sm/ie/pdcp_data_ie.c
                ../../src/sm/slice_sm/ie/slice_data_ie.c
                ../../src/util/alg_ds/alg/defer.c
                )

target_link_libraries(test_ag_ric_xapp
                      PUBLIC
                      e2_agent
                      near_ric
                      e42_iapp
                      e42_xapp
                      -pthread
                      -lsctp
                      -ldl
                      #-lnng
                      )

add_executable(test_python 
                test_python.c
                ../common/fill_ind_data.c
                ../../src/sm/mac_sm/ie/mac_data_ie.c
                ../../src/sm/rlc_sm/ie/rlc_data_ie.c
                ../../src/sm/pdcp_sm/ie/pdcp_data_ie.c
                ../../src/sm/slice_sm/ie/slice_data_ie.c
                )

target_link_libraries(test_python
                      PUBLIC
                      e2_agent
                      near_ric
                      e42_iapp
                      -pthread
                      -lsctp
                      -ldl
                      #-lnng
                      )

else()
  message(FATAL_ERROR "Only E2AP_ENCODING allowed ")
endif()
