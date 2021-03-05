add_library(flexric_agent STATIC
            e2ap_agent.c
            e2ap_endpoint_agent.c
            e2ap_msg_handler_agent.c
            e2ap_multi_control.c)
target_include_directories(flexric_agent
                           PUBLIC
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           "$<INSTALL_INTERFACE:include>")
target_link_libraries(flexric_agent
                      PUBLIC
                        types e2ap_ep e2ap_ds_lib
                      PRIVATE
                        e2ap_msg_dec e2ap_msg_enc)
if(E2AP_ENCODING STREQUAL "ASN")
  target_link_libraries(flexric_agent PUBLIC e2ap_asn1_lib)
endif()
install(FILES flexric_agent.h DESTINATION include/flexric)
