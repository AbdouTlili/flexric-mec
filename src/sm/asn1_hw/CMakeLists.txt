add_library(e2sm_hw_rf e2sm_hw_rf.c)
target_link_libraries(e2sm_hw_rf types e2sm_hw_asn1_lib flexric_agent)
target_include_directories(e2sm_hw_rf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(e2sm_hw_app e2sm_hw_app.c)
target_link_libraries(e2sm_hw_app types e2sm_hw_asn1_lib e2ap_server -lpthread)
target_include_directories(e2sm_hw_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
