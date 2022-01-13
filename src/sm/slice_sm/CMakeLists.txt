cmake_minimum_required(VERSION 3.15) # setting this is required
project(slice_sm)            # this sets the project name


set(SM_ENCODING "PLAIN" CACHE STRING "The E2AP encoding to use")
set_property(CACHE SM_ENCODING PROPERTY STRINGS "PLAIN" "ASN" "FLATBUFFERS")
message(STATUS "Selected SM_ENCODING: ${SM_ENCODING}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../util)


if(SM_ENCODING STREQUAL "PLAIN")
  add_library(slice_sm SHARED
                      ../sm_proc_data.c 
                      slice_sm_agent.c 
                      slice_sm_ric.c 
                     ../../util/byte_array.c 
                     ../../util/alg_ds/alg/defer.c 
                     ../../util/alg_ds/ds/seq_container/seq_arr.c 
                     ../../util/alg_ds/ds/seq_container/seq_ring.c 
                     ../../util/alg_ds/ds/assoc_container/assoc_rb_tree.c
                     ../../util/alg_ds/ds/assoc_container/bimap.c
                      enc/slice_enc_plain.c 
                      dec/slice_dec_plain.c 
                      ie/slice_data_ie.c 
                      )

elseif(SM_ENCODING STREQUAL "ASN" )

  add_subdirectory(ie/asn)

  add_library(slice_sm SHARED
                      ../sm_proc_data.c 
                      slice_sm_agent.c 
                      slice_sm_ric.c 
                      $<TARGET_OBJECTS:e2ap_ds_obj>
                      enc/slice_enc_asn.c 
                      dec/slice_dec_asn.c 
                      )

  target_include_directories(slice_sm  
                            PRIVATE
                            "ie/asn")

  target_compile_options(slice_sm PUBLIC "-DASN_DISABLE_OER_SUPPORT")
  target_compile_options(slice_sm PRIVATE -Wno-missing-field-initializers -Wno-unused-parameter)

elseif(SM_ENCODING STREQUAL "FLATBUFFERS" )
  add_library(slice_sm SHARED
                      ../sm_proc_data.c 
                      slice_sm_agent.c 
                      slice_sm_ric.c 
                      $<TARGET_OBJECTS:e2ap_ds_obj>
                      enc/slice_enc_fb.c 
                      dec/slice_dec_fb.c 
                      )
else()


endif()

target_compile_definitions(slice_sm PUBLIC ${SM_ENCODING})


