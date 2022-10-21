set(SM_ENCODING_SLICE "PLAIN" CACHE STRING "The SLICE SM encoding to use")
set_property(CACHE SM_ENCODING_SLICE PROPERTY STRINGS "PLAIN" "ASN" "FLATBUFFERS")
message(STATUS "Selected SLICE SM_ENCODING: ${SM_ENCODING_SLICE}")


if(SM_ENCODING_SLICE STREQUAL "PLAIN")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  add_executable(test_slice_sm
                      main.c 
                      ../../../src/sm/sm_proc_data.c 
                      ../../../src/sm/slice_sm/slice_sm_agent.c 
                      ../../../src/sm/slice_sm/slice_sm_ric.c 
                      ../../../src/sm/slice_sm/enc/slice_enc_plain.c 
                      ../../../src/sm/slice_sm/dec/slice_dec_plain.c 
                      ../../../src/util/alg_ds/alg/defer.c
                      ../../../src/util/alg_ds/alg/eq_float.c
                      ../../../src/sm/slice_sm/ie/slice_data_ie.c
                      ../common/fill_ind_data.c
              )
elseif(SM_ENCODING_SLICE STREQUAL "ASN")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}../ie/asn/ )
  file(GLOB asn_sources "../ie/asn/*.c")
  #include_directories("/usr/local/share/asn1c/")
  target_compile_options(test_slice_sm PUBLIC "-DASN_DISABLE_OER_SUPPORT")
  add_executable(test_slice_sm
                      main.c 
                      ../../sm_proc_data.c 
                      ../slice_sm_agent.c 
                      ../slice_sm_server.c 
                      ../enc/slice_enc_asn.c 
                      ../dec/slice_dec_asn.c 
                      ../../util/alg_ds/alg/defer.c
                      ../../util/alg_ds/alg/eq_float.c
                      ../ie/slice_data_ie.c
                      ../../if_sm/read/sm_rd_if.c
                      ${asn_sources} 
                      ../../../../test/common/fill_ind_data.c
              )
elseif(SM_ENCODING_SLICE STREQUAL "FLATBUFFERS")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}../ie/fb/ )
  add_executable(test_slice_sm
                      main.c 
                      ../../sm_proc_data.c 
                      ../slice_sm_agent.c 
                      ../slice_sm_server.c 
                      ../enc/slice_enc_fb.c 
                      ../dec/slice_dec_fb.c 
                      ../../../util/alg_ds/alg/defer.c
                      ../../../util/alg_ds/alg/eq_float.c
                      ../ie/slice_data_ie.c
                      ../../if_sm/read/sm_rd_if.c
                      ../../../../test/common/fill_ind_data.c
              )
endif()

#target_compile_options(test_slice_sm PRIVATE "-fno-omit-frame-pointer;-fsanitize=address;-pg;-g") # works even if -fno-rtti is set to CXX_FLAGS
target_compile_definitions(test_slice_sm PUBLIC ${SM_ENCODING_SLICE})
target_link_libraries(test_slice_sm PUBLIC -pthread)

enable_testing()
add_test(Unit_test_SLICE test_slice_sm)