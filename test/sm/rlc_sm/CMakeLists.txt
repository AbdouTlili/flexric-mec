set(SM_ENCODING_RLC "PLAIN" CACHE STRING "The RLC SM encoding to use")
set_property(CACHE SM_ENCODING_RLC PROPERTY STRINGS "PLAIN" "ASN" "FLATBUFFERS")
message(STATUS "Selected RLC SM_ENCODING: ${SM_ENCODING_RLC}")


if(SM_ENCODING_RLC  STREQUAL "PLAIN")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR} )
  add_executable(test_rlc_sm
                      main.c 
                      ../../../src/sm/sm_proc_data.c 
                      ../../../src/sm/rlc_sm/rlc_sm_agent.c 
                      ../../../src/sm/rlc_sm/rlc_sm_ric.c 
                      ../../../src/sm/rlc_sm/enc/rlc_enc_plain.c 
                      ../../../src/sm/rlc_sm/dec/rlc_dec_plain.c 
                      ../../../src/util/alg_ds/alg/defer.c
                      ../../../src/util/alg_ds/alg/eq_float.c
                      ../../../src/sm/rlc_sm/ie/rlc_data_ie.c
                      ../common/fill_ind_data.c
              )
elseif(SM_ENCODING_RLC STREQUAL "ASN")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}../ie/asn/ )
  file(GLOB asn_sources "../ie/asn/*.c")
  #include_directories("/usr/local/share/asn1c/")
  target_compile_options(test_rlc_sm PUBLIC "-DASN_DISABLE_OER_SUPPORT")
  add_executable(test_rlc_sm
                      main.c 
                      ../../sm_proc_data.c 
                      ../rlc_sm_agent.c 
                      ../rlc_sm_server.c 
                      ../enc/rlc_enc_asn.c 
                      ../dec/rlc_dec_asn.c 
                      ../../util/alg_ds/alg/defer.c
                      ../../util/alg_ds/alg/eq_float.c
                      ../ie/rlc_data_ie.c
                      ../../if_sm/read/sm_rd_if.c
                      ${asn_sources} 
                      ../../../../test/common/fill_ind_data.c
              )
elseif(SM_ENCODING_RLC STREQUAL "FLATBUFFERS")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}../ie/fb/ )
  add_executable(test_rlc_sm
                      main.c 
                      ../../sm_proc_data.c 
                      ../rlc_sm_agent.c 
                      ../rlc_sm_server.c 
                      ../enc/rlc_enc_fb.c 
                      ../dec/rlc_dec_fb.c 
                      ../../../util/alg_ds/alg/defer.c
                      ../../../util/alg_ds/alg/eq_float.c
                      ../ie/rlc_data_ie.c
                      ../../if_sm/read/sm_rd_if.c
                      ../../../../test/common/fill_ind_data.c
              )
endif()

target_compile_definitions(test_rlc_sm PUBLIC ${SM_ENCODING_RLC})
target_link_libraries(test_rlc_sm PUBLIC -pthread)

enable_testing()
add_test(Unit_test_RLC test_rlc_sm)