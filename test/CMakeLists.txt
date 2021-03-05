# we don't have a proper testing framework (yet). In the meantime, use the
# test/do_tests.py script. The below copies it to <build-dir>/test, then runs
# the above tests. Trigger it with "make test"
add_custom_target(test
  DEPENDS e2ap_test
          test_ran_mgmt
          test_cb_list
          test_sub_mgmt
          test_hash_table
  COMMAND ${CMAKE_COMMAND} -E copy
          ${CMAKE_CURRENT_SOURCE_DIR}/do_tests.py
          ${CMAKE_CURRENT_BINARY_DIR}/do_tests.py
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/do_tests.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Running tests")
add_executable(e2ap_test e2ap_test.c)

target_link_libraries(e2ap_test
                      PUBLIC flexric_agent
                             e2ap_server
                             -pthread)

add_executable(test_multi_controller test_multi_controller.c)
target_link_libraries(test_multi_controller
                      PUBLIC
                        flexric_agent
                        e2ap_server
                        pthread
                        e2sm_hw_fb_rf
                        e2sm_hw_fb_app)

add_executable(test_relay_controller test_relay_controller.c)
target_link_libraries(test_relay_controller
                      PUBLIC
                        e2ap_server
                        e2sm_hw_fb_app
                        flexric_agent
                        relay_app_rf
                        e2sm_hw_fb_rf
                        pthread)

if(E2AP_ENCODING STREQUAL "ASN")
  add_executable(test_e2ap_msg test_e2ap_msg.c)
  target_link_libraries(test_e2ap_msg PUBLIC e2ap_msg_enc e2ap_msg_dec e2ap_asn1_lib types e2ap_ds_lib)

# Test E2AP ENCODE ASN
  add_executable(e2ap_encode_asn test_e2ap_encode_asn.c)
  target_link_libraries(e2ap_encode_asn
                        PRIVATE flexric_agent
                                e2ap_server
                                e2ap_msg_enc
                                e2ap_msg_dec
                                e2ap_asn1_lib)

# Test E2AP ENCODE/DEC ASN
  add_executable(e2ap_enc_dec_asn test_e2ap_enc_dec_asn.c)
  target_link_libraries(e2ap_enc_dec_asn
                        PRIVATE flexric_agent
                                e2ap_server
                                e2ap_msg_enc
                                e2ap_msg_dec
                                e2ap_asn1_lib)

  add_dependencies(test test_e2ap_msg e2ap_encode_asn e2ap_enc_dec_asn)
elseif(E2AP_ENCODING STREQUAL "FLATBUFFERS")
# Test E2AP ENCODE/DEC Flatbuffers
  add_executable(e2ap_enc_dec_fb test_e2ap_enc_dec_fb.c)
  target_link_libraries(e2ap_enc_dec_fb
                        PRIVATE e2ap_msg_enc
                                e2ap_msg_dec
                                e2ap_encode
                                types
                                e2ap_ds_lib)
  add_dependencies(test e2ap_enc_dec_fb)
endif()



add_executable(test_ran_mgmt test_ran_mgmt.c)
target_link_libraries(test_ran_mgmt PRIVATE e2ap_server)

add_executable(test_cb_list test_cb_list.c)
target_link_libraries(test_cb_list PRIVATE e2ap_ds_lib)

add_executable(test_sub_mgmt test_sub_mgmt.c)
target_link_libraries(test_sub_mgmt PRIVATE e2ap_server)

add_executable(test_hash_table test_hash_table.c)
target_link_libraries(test_hash_table PRIVATE e2ap_ds_lib)

if(SM_HW)
  add_subdirectory(sm_asn1_hw)
endif()

if (FlatCC_FOUND)
  add_subdirectory(sm_fb_hw)
  add_subdirectory(sm_fb_mac_stats)
  add_subdirectory(sm_fb_pdcp_stats)
  add_subdirectory(sm_fb_rlc_stats)
  add_subdirectory(sm_fb_rrc_stats)
  add_subdirectory(sm_fb_rrc_conf)
  add_subdirectory(sm_fb_rrc_event)
  add_subdirectory(sm_fb_rslicing)
  add_subdirectory(flexran)
  add_subdirectory(recursive_slicing)
endif()
