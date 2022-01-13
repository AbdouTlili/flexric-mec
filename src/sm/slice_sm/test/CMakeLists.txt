cmake_minimum_required(VERSION 3.15)

project (SLICE_SM)
set(CMAKE_BUILD_TYPE Debug)
set (CMAKE_C_FLAGS "-Wall -Wextra -std=c11 -g -fsanitize=address -fno-omit-frame-pointer") 


set(SM_ENCODING "PLAIN" CACHE STRING "The E2AP encoding to use")
set_property(CACHE SM_ENCODING PROPERTY STRINGS "PLAIN" "ASN" "FLATBUFFERS")
message(STATUS "Selected SM_ENCODING: ${SM_ENCODING}")


include_directories(${CMAKE_CURRENT_SOURCE_DIR} )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}../ie/asn/ )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}../ie/fb/ )
include_directories("/usr/local/share/asn1c/")


file(GLOB asn_sources "../ie/asn/*.c")

add_executable(test_slice_sm main.c 
                      ../../sm_proc_data.c 
                      ../slice_sm_agent.c 
                      ../slice_sm_ric.c 
                      ../enc/slice_enc_plain.c 
                      ../enc/slice_enc_fb.c 
                      ../enc/slice_enc_asn.c 
                      ../dec/slice_dec_plain.c 
                      ../dec/slice_dec_fb.c 
                      ../dec/slice_dec_asn.c 
                      ../ie/slice_data_ie.c 
                      ../../../util/alg_ds/alg/defer.c
                      ../../agent_if/read/sm_ag_if_rd.c
                      ${asn_sources} 
                              )

target_compile_definitions(test_slice_sm PUBLIC ${SM_ENCODING})
target_compile_options(test_slice_sm PUBLIC "-DASN_DISABLE_OER_SUPPORT")
target_link_libraries(test_slice_sm PUBLIC -pthread)



