set(SM_ENCODING_GTP "PLAIN" CACHE STRING "The GTP SM encoding to use")
set_property(CACHE SM_ENCODING_GTP PROPERTY STRINGS "PLAIN" "ASN" "FLATBUFFERS")
message(STATUS "Selected GTP SM_ENCODING: ${SM_ENCODING_GTP}")


if(SM_ENCODING_GTP  STREQUAL "PLAIN")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR} )
  add_executable(test_gtp_sm
                      main.c 
                      ../../../src/sm/sm_proc_data.c 
                      ../../../src/sm/gtp_sm/gtp_sm_agent.c 
                      ../../../src/sm/gtp_sm/gtp_sm_ric.c 
                      ../../../src/sm/gtp_sm/enc/gtp_enc_plain.c 
                      ../../../src/sm/gtp_sm/dec/gtp_dec_plain.c 
                      ../../../src/util/alg_ds/alg/defer.c
                      ../../../src/util/alg_ds/alg/eq_float.c
                      ../../../src/sm/gtp_sm/ie/gtp_data_ie.c
                      ../common/fill_ind_data.c
              )
elseif(SM_ENCODING_GTP STREQUAL "ASN")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}../ie/asn/ )
  file(GLOB asn_sources "../ie/asn/*.c")
  #include_directories("/usr/local/share/asn1c/")
  target_compile_options(test_gtp_sm PUBLIC "-DASN_DISABLE_OER_SUPPORT")
  add_executable(test_gtp_sm
                      main.c 
                      ../../sm_proc_data.c 
                      ../gtp_sm_agent.c 
                      ../gtp_sm_server.c 
                      ../enc/gtp_enc_asn.c 
                      ../dec/gtp_dec_asn.c 
                      ../../util/alg_ds/alg/defer.c
                      ../../util/alg_ds/alg/eq_float.c
                      ../ie/gtp_data_ie.c
                      ../../if_sm/read/sm_rd_if.c
                      ${asn_sources} 
                      ../../../../test/common/fill_ind_data.c
              )
elseif(SM_ENCODING_GTP STREQUAL "FLATBUFFERS")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}../ie/fb/ )
  add_executable(test_gtp_sm
                      main.c 
                      ../../sm_proc_data.c 
                      ../gtp_sm_agent.c 
                      ../gtp_sm_server.c 
                      ../enc/gtp_enc_fb.c 
                      ../dec/gtp_dec_fb.c 
                      ../../../util/alg_ds/alg/defer.c
                      ../../../util/alg_ds/alg/eq_float.c
                      ../ie/gtp_data_ie.c
                      ../../if_sm/read/sm_rd_if.c
                      ../../../../test/common/fill_ind_data.c
              )
endif()

target_compile_definitions(test_gtp_sm PUBLIC ${SM_ENCODING_GTP})
target_link_libraries(test_gtp_sm PUBLIC -pthread)

enable_testing()
add_test(Unit_test_GTP test_gtp_sm)  
