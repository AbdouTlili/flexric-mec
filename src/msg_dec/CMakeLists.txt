if(E2AP_ENCODING STREQUAL "ASN")
  add_library(e2ap_msg_dec OBJECT e2ap_msg_free.c e2ap_msg_dec_asn.c)
elseif(E2AP_ENCODING STREQUAL "FLATBUFFERS")
  add_library(e2ap_msg_dec OBJECT e2ap_msg_free.c e2ap_msg_dec_fb.c)
else()
  message(FATAL_ERROR "no E2AP encoding scheme selected")
endif()

target_link_libraries(e2ap_msg_dec PUBLIC types e2ap_encode)
target_include_directories(e2ap_msg_dec
                           PUBLIC
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           "$<INSTALL_INTERFACE:include>")
