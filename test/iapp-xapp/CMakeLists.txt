

if(E2AP_ENCODING STREQUAL "ASN")

  if(IAPP_EMULATOR)


add_executable(test_python_iapp 
               test_python_iapp.c
               near_ric_emulator.c
                ../../src/util/alg_ds/alg/defer.c
                ../../src/sm/mac_sm/ie/mac_data_ie.c
                ../../src/sm/mac_sm/enc/mac_enc_plain.c
                ../../src/sm/rlc_sm/ie/rlc_data_ie.c
                ../../src/sm/rlc_sm/enc/rlc_enc_plain.c
                ../../src/sm/pdcp_sm/ie/pdcp_data_ie.c
                ../../src/sm/pdcp_sm/enc/pdcp_enc_plain.c
                ../common/fill_ind_data.c
               )

target_link_libraries(test_python_iapp
                      PUBLIC
                      e42_iapp
                      -pthread
                      -lsctp
                      -ldl
                      )

add_executable(test_iapp_xapp 
                test_iapp_xapp.c
                near_ric_emulator.c
                ../../src/util/alg_ds/alg/defer.c
                ../../src/sm/mac_sm/ie/mac_data_ie.c
                ../../src/sm/mac_sm/enc/mac_enc_plain.c
                ../../src/sm/rlc_sm/ie/rlc_data_ie.c
                ../../src/sm/rlc_sm/enc/rlc_enc_plain.c
                ../../src/sm/pdcp_sm/ie/pdcp_data_ie.c
                ../../src/sm/pdcp_sm/enc/pdcp_enc_plain.c
                ../common/fill_ind_data.c
                )

target_link_libraries(test_iapp_xapp
                      PUBLIC
                      e42_iapp
                      e42_xapp
                      -pthread
                      -lsctp
                      -ldl
                      )

# Set the MACRO variable that will be used by the compiler to select the encoding/decoding
target_compile_definitions(test_iapp_xapp PRIVATE ${E2AP_ENCODING})

target_include_directories(test_iapp_xapp PRIVATE "/usr/local/share/asn1c/")
target_include_directories(test_iapp_xapp PRIVATE "../src/lib/ap/ie/asn/")

target_compile_definitions(test_python_iapp PRIVATE ${E2AP_ENCODING})
target_include_directories(test_python_iapp PRIVATE "/usr/local/share/asn1c/")
target_include_directories(test_python_iapp PRIVATE "../src/lib/ap/ie/asn/")



                  else()
                    message(FATAL_ERROR "iApp only with E2AP_ENCODING implemented")
                  endif() #endif IAPP_EMULATOR



else()
  message(FATAL_ERROR "Only E2AP_ENCODING allowed ")
endif()
