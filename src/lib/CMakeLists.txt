
add_subdirectory(ap)
add_subdirectory(ep)

add_library(pending_events_obj OBJECT 
                                pending_events.c 
                                pending_event_ric.c
                                $<TARGET_OBJECTS:e2ap_types_obj> 
                                $<TARGET_OBJECTS:e2ap_ap_obj>
                                )

