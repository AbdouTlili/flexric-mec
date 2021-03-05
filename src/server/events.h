/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#ifndef EVENTS_H
#define EVENTS_H

#ifdef __cplusplus
#define EXTERNC extern "C"
#define NO_DISCARD [[nodiscard]]
#else
#define EXTERNC
#define NO_DISCARD __attribute__((warn_unused_result))
#endif

#include <stdint.h>
#include "ran_mgmt.h"
#include "cb_list.h"

typedef struct events_s {
  cb_list_t e2node_connect_cb_list;
  //cb_list_t e2node_disconnect_cb_list;
  cb_list_t ran_connect_cb_list;
  //static cb_list_t ran_disconnect_cb_list;
} events_t;

typedef enum {
  E2_NODE_CONNECT,
  //E2_NODE_DISCONNECT,
  RAN_CONNECT,
  //RAN_DISCONNECT,
} e_type;

/* Event descriptor returned by the subscribe_*() functions. Unsubscribe by
 * passing this event descriptor to unsubscribe() */
typedef struct ed_s {
  uint64_t ev_id;
  e_type type;
} ed_t;

typedef struct e2node_event_s {
  const ran_t* ran;
  int assoc_id;
} e2node_event_t;
typedef void (*e2node_connect_cb_t)(const e2node_event_t*, void*);
EXTERNC NO_DISCARD ed_t subscribe_e2node_connect(events_t* ev, e2node_connect_cb_t cb, void* data);

//typedef void (*e2node_disconnect_cb_t)(const e2node_event_t*, void*);
//EXTERNC NO_DISCARD ed_t subscribe_e2node_disconnect(events_t* ev, e2node_disconnect_cb_t cb, void* data);

typedef void (*ran_connect_cb_t)(const ran_t*, void*);
EXTERNC NO_DISCARD ed_t subscribe_ran_connect(events_t* ev, ran_connect_cb_t cb, void* data);

//typedef void (*ran_disconnect_cb_t)(const ran_t*, void*);
//EXTERNC NO_DISCARD ed_t subscribe_ran_disconnect(events_t* ev, ran_disconnect_cb_t cb, void* data);

// argument is the token returned by subscribe_xyz()
EXTERNC void unsubscribe(events_t* ev, ed_t ed);

#endif /* EVENTS_H */
