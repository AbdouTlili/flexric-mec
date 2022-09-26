
set(SM_ENCODING_MAC "PLAIN" CACHE STRING "The MAC SM encoding to use")
set_property(CACHE SM_ENCODING_MAC PROPERTY STRINGS "PLAIN" "ASN" "FLATBUFFERS")
message(STATUS "Selected MAC SM_ENCODING: ${SM_ENCODING_MAC}")


if(SM_ENCODING_MAC  STREQUAL "PLAIN")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR} )
  add_executable(test_mac_sm
                      main.c 
                      ../../../src/sm/sm_proc_data.c 
                      ../../../src/sm/mac_sm/mac_sm_agent.c 
                     ../../../src/sm/mac_sm/mac_sm_ric.c 
                     ../../../src/sm/mac_sm/enc/mac_enc_plain.c 
                     ../../../src/sm/mac_sm/dec/mac_dec_plain.c 
                     ../../../src/util/alg_ds/alg/defer.c
                     ../../../src/util/alg_ds/alg/eq_float.c
                     ../../../src/sm/mac_sm/ie/mac_data_ie.c
                      ../common/fill_ind_data.c
              )
elseif(SM_ENCODING_MAC STREQUAL "ASN")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}../ie/asn/ )
  file(GLOB asn_sources "../ie/asn/*.c")
  #include_directories("/usr/local/share/asn1c/")
  target_compile_options(test_mac_sm PUBLIC "-DASN_DISABLE_OER_SUPPORT")
  add_executable(test_mac_sm
                      main.c 
                      ../../sm_proc_data.c 
                      ../mac_sm_agent.c 
                      ../mac_sm_server.c 
                      ../enc/mac_enc_asn.c 
                      ../dec/mac_dec_asn.c 
                      ../../util/alg_ds/alg/defer.c
                      ../../util/alg_ds/alg/eq_float.c
                      ../ie/mac_data_ie.c
                      ../../if_sm/read/sm_rd_if.c
                      ../../../../test/common/fill_ind_data.c
                      ${asn_sources} 
              )
elseif(SM_ENCODING_MAC STREQUAL "FLATBUFFERS")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}../ie/fb/ )
  add_executable(test_mac_sm
                      main.c 
                      ../../sm_proc_data.c 
                      ../mac_sm_agent.c 
                      ../mac_sm_server.c 
                      ../enc/mac_enc_fb.c 
                      ../dec/mac_dec_fb.c 
                      ../../../util/alg_ds/alg/defer.c
                      ../../../util/alg_ds/alg/eq_float.c
                      ../ie/mac_data_ie.c
                      ../../if_sm/read/sm_rd_if.c
                      ../../../../test/common/fill_ind_data.c
              )
endif()

target_compile_definitions(test_mac_sm PUBLIC ${SM_ENCODING_MAC})
target_link_libraries(test_mac_sm PUBLIC -pthread)

enable_testing()
add_test(Unit_test_MAC test_mac_sm)