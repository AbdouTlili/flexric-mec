
include_directories("../src/lib/ap/ie/asn/")


if(E2AP_ENCODING STREQUAL "ASN")

####################
# E2AP ASN Encoding
####################

add_executable(test_e2ap_encode_asn test_e2ap_encode_asn.c)
target_link_libraries(test_e2ap_encode_asn
                      PUBLIC 
                      # This is wrong. Fix It
                      e2_agent
                      $<TARGET_OBJECTS:e2ap_ie_obj>
                      )
target_compile_options(test_e2ap_encode_asn  PUBLIC "-DASN_DISABLE_OER_SUPPORT")
target_compile_options(test_e2ap_encode_asn  PRIVATE -Wno-missing-field-initializers -Wno-unused-parameter)


add_executable(test_e2ap_enc_dec_asn test_e2ap_enc_dec_asn.c)
target_link_libraries(test_e2ap_enc_dec_asn
                      PUBLIC 
                      # This is wrong. Fix It
                      e2_agent
                      $<TARGET_OBJECTS:e2ap_ie_obj>
                      )
target_compile_options(test_e2ap_enc_dec_asn PUBLIC "-DASN_DISABLE_OER_SUPPORT")
target_compile_options(test_e2ap_enc_dec_asn PRIVATE -Wno-missing-field-initializers -Wno-unused-parameter)


###########################
# E2AP Flatbuffers Encoding
###########################

elseif(E2AP_ENCODING STREQUAL "FLATBUFFERS")

  add_executable(test_e2ap_enc_dec_fb test_e2ap_enc_dec_fb.c)

  target_link_libraries(test_e2ap_enc_dec_fb
                      PUBLIC 
                      # This is wrong. Fix It
                      e2_agent
                      ${FlatCC}
                      )
  target_compile_definitions(test_e2ap_enc_dec_fb  PRIVATE ${E2AP_ENCODING})

endif()


#############################
# E2E Test E2-AGENT NEAR-RIC 
#############################


add_executable(test_near_ric test_near_ric.c )
target_link_libraries(test_near_ric
                      PUBLIC 
                      e2_agent
                      ${FlatCC} 
                      #/home/mir/workspace/flatcc/lib/libflatccrt_d.a
                      near_ric
                      #ToDO: get rid of nng dependency
                      -lnng
                      -pthread
                      -lsctp
                      -ldl)

add_executable(test_agent test_agent.c)
target_link_libraries(test_agent
                      PUBLIC 
                      e2_agent
                      ${FlatCC} 
                      #ToDO: get rid of nng dependency
                      -lnng
                      -pthread
                      -lsctp
                      -ldl)

add_executable(near_ric_sa ric_sa.c )
target_link_libraries(near_ric_sa
                      PUBLIC 
                      near_ric
                      ${FlatCC} 
                      #ToDO: get rid of nng dependency
                      -lnng
                      -pthread
                      -lsctp
                      -ldl)





