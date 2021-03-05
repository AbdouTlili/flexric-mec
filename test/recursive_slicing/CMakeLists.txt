add_executable(test_res_virt test_res_virt.cc)
target_link_libraries(test_res_virt res_virt)

add_executable(test_nvs test_nvs.cc)
target_link_libraries(test_nvs res_virt)
