add_library(e2ap_server STATIC
            e2ap_server.c
            e2ap_endpoint_ric.c
            e2ap_msg_handler_ric.c
            ran_mgmt.c
            events.c
            sub_mgmt.c
            )

target_include_directories(e2ap_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(e2ap_server PUBLIC e2ap_ep e2ap_msg_dec e2ap_msg_enc e2ap_encode e2ap_ds_lib)
if(E2AP_ENCODING STREQUAL "ASN")
  target_link_libraries(e2ap_server PUBLIC e2ap_asn1_lib)
endif()

add_executable(flexric_rtc flexric_rtc.c)
target_link_libraries(flexric_rtc e2ap_server)
