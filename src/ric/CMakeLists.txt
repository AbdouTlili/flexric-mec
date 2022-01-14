if(BUILDING_LIBRARY STREQUAL "STATIC")
  set(RIC_BLD_LIB  "STATIC")
elseif(BUILDING_LIBRARY STREQUAL "DYNAMIC")
  set(RIC_BLD_LIB  "SHARED")
else()
  message(FATAL_ERROR "Unknown type")
endif()


set(RIC_SRC 
            e2ap_ric.c
            e2_node.c
            asio_ric.c
            endpoint_ric.c
            msg_handler_ric.c
            near_ric.c
            near_ric_api.c
            iApps/redis.c
            iApps/stdout.c
            iApps/influx.c
            plugin_ric.c
            iApps/string_parser.c
            iApps/nng/notify_nng_listener.c
            iApps/nng/req_reply.c
            iApps/nng/keep_alive/keep_alive_timer.c
            $<TARGET_OBJECTS:e2ap_ep_obj> 
            $<TARGET_OBJECTS:e2ap_ap_obj>
            $<TARGET_OBJECTS:e2ap_ds_obj>
            $<TARGET_OBJECTS:e2ap_alg_obj>
            $<TARGET_OBJECTS:e2_conf_obj>
            $<TARGET_OBJECTS:e2ap_msg_enc_obj>
            $<TARGET_OBJECTS:e2ap_msg_dec_obj>
            $<TARGET_OBJECTS:e2ap_msg_free_obj>
            $<TARGET_OBJECTS:e2ap_types_obj> 
            $<TARGET_OBJECTS:pending_events_obj>
            )

if(E2AP_ENCODING STREQUAL "ASN")
  add_library(near_ric ${RIC_BLD_LIB}# STATIC or SHARED
            ${RIC_SRC}
            $<TARGET_OBJECTS:e2ap_asn1_obj>
            )
elseif(E2AP_ENCODING STREQUAL "FLATBUFFERS" )
  add_library(near_ric ${RIC_BLD_LIB} # STATIC or SHARED
            ${RIC_SRC}
            )
endif()

if(INCL_WHAT_YOU_USE)
  find_program(iwyu_path NAMES include-what-you-use iwyu REQUIRED)
  set_property(TARGET near_ric PROPERTY C_INCLUDE_WHAT_YOU_USE ${iwyu_path})
endif()


target_compile_definitions(near_ric PRIVATE ${E2AP_ENCODING})

