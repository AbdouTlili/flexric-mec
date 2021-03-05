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

#ifndef RRC_EVENT_APP_H
#define RRC_EVENT_APP_H

#include "e2ap_server.h"
#include "type_defs.h"
#include "cb_list.h"

typedef struct e2sm_rrc_event_app_s {
  e2ap_ric_t* ric;
  ric_gen_id_t ric_id;

  ed_t e2node_subscription;

  ran_t* ran;
  int assoc_id;

  cb_list_t ue_attach;
  cb_list_t ue_conn_complete;
  cb_list_t ue_release;
} e2sm_rrc_event_app_t;

void e2sm_rrc_event_app_init(e2sm_rrc_event_app_t* app, e2ap_ric_t* ric, uint16_t ric_req_id);
void e2sm_rrc_event_app_free(e2sm_rrc_event_app_t* app);

#ifdef __cplusplus
#define EXTERNC extern "C"
#define NO_DISCARD [[nodiscard]]
#else
#define EXTERNC
#define NO_DISCARD __attribute__((warn_unused_result))
#endif

typedef enum {
  UE_ATTACH,
  UE_CONN_COMPLETE,
  UE_RELEASE
} e_ue_type;

/* Event descriptor returned by the subscribe_ue_*() functions. Unsubscribe by
 * passing this event descriptor to unsubscribe_ue() */
typedef struct ue_ed_s {
  uint64_t ev_id;
  e_ue_type type;
} ue_ed_t;

// msg is flatbuffer rrc_event_IndicationMessage (root)
typedef void (*ue_event_cb_t)(const ran_t*, uint16_t rnti, byte_array_t msg, void* data);

EXTERNC NO_DISCARD
ue_ed_t subscribe_ue_attach(e2sm_rrc_event_app_t *app, ue_event_cb_t cb, void* data);
EXTERNC NO_DISCARD
ue_ed_t subscribe_ue_conn_complete(e2sm_rrc_event_app_t *app, ue_event_cb_t cb, void* data);
EXTERNC NO_DISCARD
ue_ed_t subscribe_ue_release(e2sm_rrc_event_app_t *app, ue_event_cb_t cb, void* data);

// argument is the token returned by subscribe_ue_xyz()
EXTERNC void unsubscribe_ue(e2sm_rrc_event_app_t* ev, ue_ed_t ed);

#endif /* RRC_EVENT_APP_H */
