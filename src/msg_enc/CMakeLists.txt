if(E2AP_ENCODING STREQUAL "ASN")
  add_library(e2ap_msg_enc OBJECT e2ap_msg_enc_asn.c)
elseif(E2AP_ENCODING STREQUAL "FLATBUFFERS")
  add_library(e2ap_msg_enc OBJECT e2ap_msg_enc_fb.c)
endif()
target_link_libraries(e2ap_msg_enc PUBLIC types e2ap_encode)
target_include_directories(e2ap_msg_enc
                           PUBLIC
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           "$<INSTALL_INTERFACE:include>")
