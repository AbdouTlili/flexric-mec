set(SM_ENCODING_PDCP "PLAIN" CACHE STRING "The PDCP SM encoding to use")
set_property(CACHE SM_ENCODING_PDCP PROPERTY STRINGS "PLAIN" "ASN" "FLATBUFFERS")
message(STATUS "Selected PDCP SM_ENCODING: ${SM_ENCODING_PDCP}")


if(SM_ENCODING_PDCP  STREQUAL "PLAIN")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR} )
  add_executable(test_pdcp_sm
                      main.c 
                      ../../../src/sm/sm_proc_data.c  
                      ../../../src/sm/pdcp_sm/pdcp_sm_agent.c 
                      ../../../src/sm/pdcp_sm/pdcp_sm_ric.c 
                      ../../../src/sm/pdcp_sm/enc/pdcp_enc_plain.c 
                      ../../../src/sm/pdcp_sm/dec/pdcp_dec_plain.c 
                      ../../../src/util/alg_ds/alg/defer.c
                      ../../../src/util/alg_ds/alg/eq_float.c
                      ../../../src/sm/pdcp_sm/ie/pdcp_data_ie.c
                      ../common/fill_ind_data.c
              )
elseif(SM_ENCODING_PDCP STREQUAL "ASN")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}../ie/asn/ )
  file(GLOB asn_sources "../ie/asn/*.c")
  #include_directories("/usr/local/share/asn1c/")
  target_compile_options(test_pdcp_sm PUBLIC "-DASN_DISABLE_OER_SUPPORT")
  add_executable(test_pdcp_sm
                      main.c 
                      ../../sm_proc_data.c 
                      ../pdcp_sm_agent.c 
                      ../pdcp_sm_server.c 
                      ../enc/pdcp_enc_asn.c 
                      ../dec/pdcp_dec_asn.c 
                      ../../util/alg_ds/alg/defer.c
                      ../../util/alg_ds/alg/eq_float.c
                      ../ie/pdcp_data_ie.c
                      ../../if_sm/read/sm_rd_if.c
                      ${asn_sources} 
                      ../../../../test/common/fill_ind_data.c
              )
elseif(SM_ENCODING_PDCP STREQUAL "FLATBUFFERS")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}../ie/fb/ )
  add_executable(test_pdcp_sm
                      main.c 
                      ../../sm_proc_data.c 
                      ../pdcp_sm_agent.c 
                      ../pdcp_sm_server.c 
                      ../enc/pdcp_enc_fb.c 
                      ../dec/pdcp_dec_fb.c 
                      ../../../util/alg_ds/alg/defer.c
                      ../../../util/alg_ds/alg/eq_float.c
                      ../ie/pdcp_data_ie.c
                      ../../if_sm/read/sm_rd_if.c
                      ../../../../test/common/fill_ind_data.c
              )
endif()

target_compile_definitions(test_pdcp_sm PUBLIC ${SM_ENCODING_PDCP})
target_link_libraries(test_pdcp_sm PUBLIC -pthread)

enable_testing()
add_test(Unit_test_PDCP test_pdcp_sm)