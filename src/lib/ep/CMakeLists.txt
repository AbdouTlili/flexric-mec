add_library(e2ap_ep_obj OBJECT e2ap_ep.c )
#target_include_directories(e2ap_ep
#                           PUBLIC
#                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
#                           "$<INSTALL_INTERFACE:include>")

target_link_libraries(e2ap_ep_obj PRIVATE -lsctp)


#########################################
if(FALSE)
add_library(e2ap_encode INTERFACE)
target_include_directories(e2ap_encode
                           INTERFACE
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           "$<INSTALL_INTERFACE:include>")
if(E2AP_ENCODING STREQUAL "ASN")
  target_compile_options(e2ap_encode INTERFACE "-DASN")
  target_link_libraries(e2ap_encode INTERFACE e2ap_asn1_lib)
elseif(E2AP_ENCODING STREQUAL "FLATBUFFERS")
  target_compile_options(e2ap_encode INTERFACE "-DFLATBUFFERS")
  target_link_libraries(e2ap_encode INTERFACE e2ap_fb_lib)
endif()

endif()



