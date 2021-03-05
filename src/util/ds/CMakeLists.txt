add_library(e2ap_ds_lib OBJECT byte_array.c rb_tree.c cb_list.c hash_table.c)
target_include_directories(e2ap_ds_lib
                           PUBLIC
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           "$<INSTALL_INTERFACE:include>")
install(FILES rb_tree.h byte_array.h DESTINATION include/flexric)
