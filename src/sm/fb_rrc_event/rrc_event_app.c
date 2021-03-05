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

#include <assert.h>
#include <stdio.h>

#include "rrc_event_app.h"
#include "rrc_event_defs.h"
#include "rrc_event_msg_enc.h"
#include "rrc_event_msg_dec.h"
#include "rrc_event_reader.h"

static
void rrc_event_handle_subscription_response(const ric_subscription_response_t *r, void* data)
{
  assert(r);
  assert(data);
  e2sm_rrc_event_app_t* app = (e2sm_rrc_event_app_t*) data;
  //printf("[E2SM_RRC_UE_EVENT] received subscription response for RAN %lx\n",
  //       app->ran->generated_ran_key);
}

static
void rrc_event_handle_subscription_failure(const ric_subscription_failure_t* f, void* data)
{
  assert(f);
  assert(data);
  e2sm_rrc_event_app_t* app = (e2sm_rrc_event_app_t*) data;

  //printf("[E2SM_RRC_UE_EVENT] received subscription failure for RAN %lx, dropping it\n",
  //       app->ran->generated_ran_key);
  app->ran = NULL;
}

static
void rrc_event_handle_indication(const ric_indication_t* i, void* data)
{
  assert(i);
  assert(data);
  e2sm_rrc_event_app_t* app = (e2sm_rrc_event_app_t*) data;

  rrc_event_indication_header_t hdr = rrc_event_decode_indication_header(i->hdr);

  //printf("[E2SM_RRC_UE_EVENT] received indication from RAN %lx: RNTI %04x event %s\n",
  //       app->ran->generated_ran_key, hdr.rnti, rrc_event_Event_name(hdr.event));

  void trigger_cb(void* cb, void* signal_data, void* user_data) {
    ue_event_cb_t real_cb = (ue_event_cb_t) cb;
    assert(!signal_data); // local function: use variables in rrc_event_handle_indication()
    real_cb(app->ran, hdr.rnti, i->msg, user_data);
  };

  switch (hdr.event) {
    case rrc_event_Event_Attach:
      cb_list_foreach(&app->ue_attach, trigger_cb, NULL);
      break;
    case rrc_event_Event_Complete:
      cb_list_foreach(&app->ue_conn_complete, trigger_cb, NULL);
      break;
    case rrc_event_Event_Release:
      cb_list_foreach(&app->ue_release, trigger_cb, NULL);
      break;
    default:
      assert(0 && "illegal UE event");
      break;
  }
}

static
void rrc_event_handle_subscription_delete_response(const ric_subscription_delete_response_t* dr, void *data)
{
  assert(dr);
  assert(data);
  e2sm_rrc_event_app_t* app = (e2sm_rrc_event_app_t*) data;
  //printf("[E2SM_RRC_UE_EVENT] received subscription delete response\n");
  app->ran = NULL;
}

static
void rrc_event_handle_subscription_delete_failure(const ric_subscription_delete_failure_t* df, void* data)
{
  assert(df && data);
  //printf("[E2SM_RRC_UE_EVENT] received subscription delete failure\n");
  assert(0 && "should not happen");
}

static
void e2sm_rrc_event_subscribe(const e2node_event_t* ev, void *data)
{
  assert(ev && data);
  e2sm_rrc_event_app_t* app = (e2sm_rrc_event_app_t*) data;
  if (app->ran) {
    //printf("[E2SM_RRC_UE_EVENT] new E2 node RAN %lx assoc_id %d, but already active RAN %lx and do not handle multiple RANs yet\n",
    //       ev->ran->generated_ran_key, ev->assoc_id, app->ran->generated_ran_key);
    return;
  }

  const entity_info_t* e = ran_mgmt_get_entity(ev->ran, ev->assoc_id);
  for (size_t i = 0; i < e->e2_setup_request.len_rf; ++i) {
    const ran_function_t* rf = &e->e2_setup_request.ran_func_item[i];
    if (strncmp((char*) rf->oid->buf, rrc_event_oid, rf->oid->len) != 0)
      continue;

    //printf("[E2SM_RRC_UE_EVENT] new E2 node RAN %lx assoc_id %d has rrc event SM on RAN function ID %d\n",
    //       ev->ran->generated_ran_key, ev->assoc_id, rf->id);

    app->ran = (ran_t*) ev->ran;
    app->assoc_id = ev->assoc_id;
    app->ric_id.ran_func_id = rf->id;

    byte_array_t event = rrc_event_encode_event_trigger(rrc_event_ReportOccasion_onChange);
    rrc_event_report_style_t style = { .type = rrc_event_ReportStyleType_Minimal };
    byte_array_t act_def = rrc_event_encode_action_definition(style);
    ric_action_t action = {
      .id = 0,
      .type = RIC_ACT_REPORT,
      //.definition = &act_def
    };
    ric_subscription_request_t sr = {
      .ric_id = app->ric_id,
      .event_trigger = event,
      .action = &action,
      .len_action = 1
    };
    cb_sub_t cb = {
      .sub_resp = rrc_event_handle_subscription_response,
      .sub_fail = rrc_event_handle_subscription_failure,
      .ind = rrc_event_handle_indication
    };
    e2ap_subscribe(app->ric, ev->assoc_id, &sr, cb, app);
    free_byte_array(event);
    free_byte_array(act_def);
    return;
  }
}

static
void e2sm_rrc_event_unsubscribe(e2sm_rrc_event_app_t* app)
{
  assert(app && app->ric && app->ran && app->assoc_id > 0);

  const ric_subscription_delete_request_t sdr = { .ric_id = app->ric_id };
  cb_sub_del_t cb = {
    .sub_del_resp = rrc_event_handle_subscription_delete_response,
    .sub_del_fail = rrc_event_handle_subscription_delete_failure
  };
  e2ap_unsubscribe(app->ric, app->assoc_id, &sdr, cb);
}

void e2sm_rrc_event_app_init(e2sm_rrc_event_app_t* app, e2ap_ric_t* ric, uint16_t ric_req_id)
{
  assert(app);
  assert(ric);

  app->ric = ric;
  app->ran = NULL;
  app->assoc_id = 0;
  app->ric_id.ric_req_id = ric_req_id;
  app->ric_id.ric_inst_id = 0;
  app->ric_id.ran_func_id = 0;

  cb_list_create(&app->ue_attach);
  cb_list_create(&app->ue_conn_complete);
  cb_list_create(&app->ue_release);

  ed_t ed = subscribe_e2node_connect(&ric->events, e2sm_rrc_event_subscribe, app);
  app->e2node_subscription = ed;
  //printf("[E2SM_RRC_UE_EVENT] app init done\n");
}

void e2sm_rrc_event_app_free(e2sm_rrc_event_app_t* app)
{
  assert(app);
  cb_list_destroy(&app->ue_attach);
  cb_list_destroy(&app->ue_conn_complete);
  cb_list_destroy(&app->ue_release);

  if (app->ran) {
    e2sm_rrc_event_unsubscribe(app);
    while (app->ran)
      usleep(100000);
    app->assoc_id = 0;
  }

  unsubscribe(&app->ric->events, app->e2node_subscription);
  app->ric = NULL;
  //printf("[E2SM_RRC_UE_EVENT] app free done\n");
}

ue_ed_t subscribe_ue_attach(e2sm_rrc_event_app_t *app, ue_event_cb_t cb, void* data)
{
  assert(app);
  return (ue_ed_t) {
    .ev_id = cb_list_subscribe(&app->ue_attach, (void*) cb, data),
    .type = UE_ATTACH
  };
}

ue_ed_t subscribe_ue_conn_complete(e2sm_rrc_event_app_t *app, ue_event_cb_t cb, void* data)
{
  assert(app);
  return (ue_ed_t) {
    .ev_id = cb_list_subscribe(&app->ue_conn_complete, (void*) cb, data),
    .type = UE_CONN_COMPLETE
  };
}

ue_ed_t subscribe_ue_release(e2sm_rrc_event_app_t *app, ue_event_cb_t cb, void* data)
{
  assert(app);
  return (ue_ed_t) {
    .ev_id = cb_list_subscribe(&app->ue_release, (void*) cb, data),
    .type = UE_RELEASE
  };
}

void unsubscribe_ue(e2sm_rrc_event_app_t* app, ue_ed_t ed)
{
  assert(app);
  if (ed.type == UE_ATTACH)
    cb_list_unsubscribe(&app->ue_attach, ed.ev_id);
  else if (ed.type == UE_CONN_COMPLETE)
    cb_list_unsubscribe(&app->ue_conn_complete, ed.ev_id);
  else if (ed.type == UE_RELEASE)
    cb_list_unsubscribe(&app->ue_release, ed.ev_id);
  else
    assert(0 && "bad ed value");
}
