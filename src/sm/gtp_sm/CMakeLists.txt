cmake_minimum_required(VERSION 3.15) # setting this is required
project(gtp_sm)            # this sets the project name

set(SM_ENCODING_GTP_SRC
                      ../sm_proc_data.c 
                      gtp_sm_agent.c 
                      gtp_sm_ric.c 
                     ../../util/byte_array.c 
                     ../../util/alg_ds/alg/defer.c 
                     ../../util/alg_ds/alg/eq_float.c 
                     ../../util/alg_ds/ds/seq_container/seq_arr.c 
                     ../../util/alg_ds/ds/seq_container/seq_ring.c 
                     ../../util/alg_ds/ds/assoc_container/assoc_rb_tree.c
                     ../../util/alg_ds/ds/assoc_container/bimap.c
                      ie/gtp_data_ie.c 
                      )


if(SM_ENCODING_GTP STREQUAL "PLAIN")
  add_library(gtp_sm SHARED
                      ${SM_ENCODING_GTP_SRC}
                      enc/gtp_enc_plain.c 
                      dec/gtp_dec_plain.c 
                      )
else()
  message(FATAL_ERROR "Unknown SM encoding type ")
endif()

target_compile_definitions(gtp_sm PUBLIC ${SM_ENCODING_GTP})

