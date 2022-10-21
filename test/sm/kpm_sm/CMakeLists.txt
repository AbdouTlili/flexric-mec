cmake_minimum_required(VERSION 3.15)

project (TEST_KPM_SM)
set(CMAKE_BUILD_TYPE Debug)
set (CMAKE_C_FLAGS "-Wall -Wextra --std=gnu11 -g -fno-omit-frame-pointer") 
set(SM_ENCODING_KPM "PLAIN" CACHE STRING "The SM encoding to use")
set_property(CACHE SM_ENCODING_KPM PROPERTY STRINGS "PLAIN" "ASN" "FLATBUFFERS")
message(STATUS "Selected SM_ENCODING_KPM: ${SM_ENCODING_KPM} for tests")

if(SM_ENCODING_KPM STREQUAL "ASN")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}../ie/asn/ )
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  file(GLOB asn_sources "../../../src/sm/kpm_sm_v2.02/ie/asn/*.c")
  add_executable(test_kpm_sm 
                      main.c 
                      ../../../src/sm/sm_proc_data.c 
                      ../../../src/sm/kpm_sm_v2.02/kpm_sm_agent.c 
                      ../../../src/sm/kpm_sm_v2.02/kpm_sm_ric.c 
                      ../../../src/sm/kpm_sm_v2.02/enc/kpm_enc_asn.c 
                      ../../../src/sm/kpm_sm_v2.02/dec/kpm_dec_asn.c 
                      ../../../src/sm/kpm_sm_v2.02/ie/kpm_data_ie.c 
                      ../../../src/util/alg_ds/alg/defer.c
                      ../../../src/util/alg_ds/alg/eq_float.c
                      ../../../src/util/byte_array.c
                      ../common/fill_ind_data.c
                      ${asn_sources}
                      )
                        
  # XXX-IMPROVE: maybe we can avoid recompiling objects like asn_sources. 
  target_compile_options(test_kpm_sm PUBLIC "-DASN_DISABLE_OER_SUPPORT")
  target_compile_options(test_kpm_sm PRIVATE -Wno-missing-field-initializers -Wno-unused-parameter)
  target_include_directories(test_kpm_sm PRIVATE "../../../src/sm/kpm_sm_v2.02/ie/asn")
else()
  message(FATAL_ERROR "unsupported encoding type")
endif()

target_compile_definitions(test_kpm_sm PUBLIC ${SM_ENCODING_KPM})
target_link_libraries(test_kpm_sm PUBLIC -pthread)
target_link_libraries(test_kpm_sm PUBLIC m)

enable_testing()
add_test(Unit_test_KPM test_kpm_sm)
