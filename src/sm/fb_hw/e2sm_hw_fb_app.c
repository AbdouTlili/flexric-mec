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
#include <errno.h>
#include <stdio.h>

#include "e2sm_hw_fb_app.h"

#include "e2sm_hw_builder.h"
#include "e2sm_hw_verifier.h"

static
uint32_t get_usec_diff(struct timeval start, struct timeval end)
{
  uint32_t start_usec = start.tv_sec * 1000000 + start.tv_usec;
  uint32_t end_usec = end.tv_sec * 1000000 + end.tv_usec;
  return end_usec - start_usec;
}

static
void handle_indication(const ric_indication_t *i, void* data)
{
  assert(i && data);
  e2sm_hw_fb_app_t* app = (e2sm_hw_fb_app_t*) data;

  struct timeval now;
  gettimeofday(&now, NULL);
  const uint32_t usec_rtt = get_usec_diff(app->control_sent, now);

  const long h = e2sm_hw_fb_decode_indication_header(i->hdr);
  if (h == app->count) {
    printf("[E2SM_HW_FB_APP] received expected indication no %ld after %d usec\n", h, usec_rtt);
    fflush(stdout);
  } else {
    printf("[E2SM_HW_FB_APP] received indication no %ld after %d usec, but expected no %d\n",
           h, usec_rtt, app->count);
    assert(0 && "unexpected indication number");
  }
}

static
void send_control(e2ap_ric_t* ric, int tfd, void* data)
{
  assert(ric);
  assert(data);
  e2sm_hw_fb_app_t* app = (e2sm_hw_fb_app_t*) data;
  assert(app->fd == tfd);

  app->count = 0;

  gettimeofday(&app->control_sent, NULL);
  app->count += 1;

  char *ctrl_text = "Hello from E2SM-HW-FB, this is a control";
  byte_array_t ctrl_msg = { .buf = (uint8_t*) ctrl_text, .len = strlen(ctrl_text) };
  byte_array_t msg = e2sm_hw_fb_generate_control_message(ctrl_msg);
  byte_array_t hdr = e2sm_hw_fb_generate_control_header(app->count);
  ric_control_request_t ric_req = { .ric_id = app->ric_id, .hdr = hdr, .msg = msg, };

  void ctrl_ack(const ric_control_acknowledge_t* a, void* data) { assert(a && !data); }
  void ctrl_fail(const ric_control_failure_t* f, void* data) { assert(f && !data); }
  cb_ctrl_t cbr = {
    .ctrl_ack = ctrl_ack,
    .ctrl_fail = ctrl_fail
  };
  e2ap_control_req(app->server, app->assoc_id, &ric_req, cbr, NULL);
  free(msg.buf);
  free(hdr.buf);
}

static
void sub_resp_cb(const ric_subscription_response_t *r, void *data)
{
  assert(r && data);
  e2sm_hw_fb_app_t* app = (e2sm_hw_fb_app_t*) data;

  printf("[E2SM_HW_FB_APP] received subscription response, start RTT ping thread\n");

  int ms = app->ping_interval_ms;
  app->fd = e2ap_add_timer_epoll_ms_server(app->server, ms, ms, send_control, app);

  if (app->fd >= 0) {
    printf("[E2SM_HW_FB_APP] created event to send control every %d ms\n", app->ping_interval_ms);
  } else {
    printf("[E2SM_HW_FB_APP] could not create event\n");
    app->ran = NULL;
    app->assoc_id = -1;
  }
}

static
void sub_fail_cb(const ric_subscription_failure_t* r, void *data)
{
  assert(r && data);
  e2sm_hw_fb_app_t* app = (e2sm_hw_fb_app_t*) data;

  printf("[E2SM_HW_FB_APP] received subscription failure for RAN %lx assoc ID %d, dropping it\n",
         app->ran->generated_ran_key, app->assoc_id);
  app->ran = NULL;
  app->assoc_id = -1;
}

static
void subscribe(const e2node_event_t* ev, void *data)
{
  assert(ev && data);
  e2sm_hw_fb_app_t* app = (e2sm_hw_fb_app_t*) data;

  if (app->ran) {
    printf("[E2SM_HW_FB_APP] new E2 node RAN %lx assoc_id %d, but already active RAN %lx assoc_id %d\n",
           ev->ran->generated_ran_key, ev->assoc_id, app->ran->generated_ran_key, app->assoc_id);
    return;
  }

  const entity_info_t* e = ran_mgmt_get_entity(ev->ran, ev->assoc_id);
  for (size_t i = 0; i < e->e2_setup_request.len_rf; ++i) {
    const ran_function_t* rf = &e->e2_setup_request.ran_func_item[i];
    if (strncmp((char*) rf->def.buf, "This is the E2SM-HW-FB", rf->def.len) != 0)
      continue;

    printf("[E2SM_HW_FB_APP] new E2 node RAN %lx assoc_id %d has E2SM-HW-FB on RAN function ID %d\n",
           ev->ran->generated_ran_key, ev->assoc_id, rf->id);
    app->ran = (ran_t*) ev->ran;
    app->assoc_id = ev->assoc_id;
    app->ric_id.ran_func_id = rf->id;

    byte_array_t event = e2sm_hw_fb_generate_event_trigger_definition(hw_TriggerNature_OnChange);
    ric_action_t action = { .id = 42, .type = RIC_ACT_REPORT };
    ric_subscription_request_t sr = {
      .ric_id = app->ric_id,
      .event_trigger = event,
      .action = &action,
      .len_action = 1
    };

    cb_sub_t cb = {
      .sub_resp = sub_resp_cb,
      .sub_fail = sub_fail_cb,
      .ind = handle_indication
    };

    e2ap_subscribe(app->server, app->assoc_id, &sr, cb, app);
    free(event.buf);
    return;
  }
  printf("[E2SM_HW_FB_APP] new E2 node RAN %lx assoc_id %d, but no RF with E2SM-HW-FB present\n",
         ev->ran->generated_ran_key, ev->assoc_id);
}

static
void sub_del_resp_cb(const ric_subscription_delete_response_t* dr, void *data)
{
  assert(dr && data);
  e2sm_hw_fb_app_t* app = (e2sm_hw_fb_app_t*) data;
  printf("[E2SM_HW_FB_APP] received subscription delete response\n");
  app->ran = NULL;
  app->assoc_id = -1;
}

static
void sub_del_fail_cb(const ric_subscription_delete_failure_t* df, void *data)
{
  assert(df && data);
  e2sm_hw_fb_app_t* app = (e2sm_hw_fb_app_t*) data;
  printf("[E2SM_HW_FB_APP] received subscription delete failure from assoc_id %d\n", app->assoc_id);
  assert(0 && "should not happen");
}

static
void e2sm_hw_fb_unsubscribe(e2sm_hw_fb_app_t* app)
{
  assert(app);
  assert(app->server);
  assert(app->ran && app->assoc_id >= 0 && "app should be subscribed");

  const ric_subscription_delete_request_t sd = { .ric_id = app->ric_id };
  cb_sub_del_t cb = {
    .sub_del_resp = sub_del_resp_cb,
    .sub_del_fail = sub_del_fail_cb
  };
  e2ap_unsubscribe(app->server, app->assoc_id, &sd, cb);
}

void e2sm_hw_fb_app_init(e2sm_hw_fb_app_t* app, e2ap_ric_t* server, unsigned int ping_interval_ms)
{
  assert(app);
  assert(server);

  app->server = server;

  app->ran = NULL;
  app->assoc_id = -1;
  app->ric_id.ric_req_id = 11;
  app->ric_id.ric_inst_id = 12;

  app->ping_interval_ms = ping_interval_ms;
  app->fd = -1;
  app->control_sent.tv_sec = app->control_sent.tv_usec = 0;
  app->count = 0;

  ed_t ed = subscribe_e2node_connect(&server->events, subscribe, app);
  (void) ed;
  printf("[E2SM_HW_FB_APP] init app done\n");
}

void e2sm_hw_fb_app_free(e2sm_hw_fb_app_t* app)
{
  printf("[E2SM_HW_FB_APP] free app start\n");
  assert(app);
  if (app->fd >= 0)
    e2ap_remove_timer_epoll_server(app->fd);

  if (app->ran) {
    e2sm_hw_fb_unsubscribe(app);
    while (app->ran)
      usleep(10000);
  }
  printf("[E2SM_HW_FB_APP] free app done\n");
}

byte_array_t e2sm_hw_fb_generate_action_definition(void)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  hw_ActionDefinitionFormat1_start(B);
  flatbuffers_string_ref_t str = flatbuffers_string_create_str(B, "1337");
  hw_ActionDefinitionFormat1_ranParameterList_start(B);
    hw_ActionDefinitionFormat1_ranParameterList_push_create(B,
        12,
        str,
        hw_RanParameterTest_contains,
        str);
  hw_ActionDefinitionFormat1_ranParameterList_end(B);
  hw_ActionDefinitionFormat1_ref_t def = hw_ActionDefinitionFormat1_end(B);
  hw_ActionDefinitionUnion_union_ref_t u = hw_ActionDefinitionUnion_as_actionDefF1(def);
  hw_ActionDefinition_create_as_root(B, u);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);

  return ba;
}

/* E2SM-HW Event trigger */
byte_array_t e2sm_hw_fb_generate_event_trigger_definition(hw_TriggerNature_enum_t n)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  hw_EventTriggerDefinitionFormat1_ref_t def = hw_EventTriggerDefinitionFormat1_create(B, n);
  hw_EventTriggerUnion_union_ref_t u = hw_EventTriggerUnion_as_eventDefF1(def);
  hw_EventTrigger_create_as_root(B, u);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t e2sm_hw_fb_decode_indication_message(byte_array_t b)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = hw_IndicationMessage_verify_as_root(b.buf, b.len))) {
    printf("EventTrigger is invalid: %s\n", flatcc_verify_error_string(ret));
    assert(0);
  }
  hw_IndicationMessage_table_t im = hw_IndicationMessage_as_root(b.buf);
  hw_IndicationMessageUnion_union_t u = hw_IndicationMessage_msg_union(im);
  assert(u.type == hw_IndicationMessageUnion_indMessageF1);
  hw_IndicationMessageFormat1_table_t imf1 = u.value;
  const uint8_t* p = hw_IndicationMessageFormat1_param(imf1);
  const size_t pl = flatbuffers_uint8_vec_len(p);
  byte_array_t bret = { .buf = malloc(pl), .len = pl };
  assert(bret.buf);
  memcpy(bret.buf, p, pl);
  return bret;
}

uint32_t e2sm_hw_fb_decode_indication_header(byte_array_t b)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = hw_IndicationHeader_verify_as_root(b.buf, b.len))) {
    printf("EventTrigger is invalid: %s\n", flatcc_verify_error_string(ret));
    assert(0);
  }
  hw_IndicationHeader_table_t im = hw_IndicationHeader_as_root(b.buf);
  hw_IndicationHeaderUnion_union_t u = hw_IndicationHeader_hdr_union(im);
  assert(u.type == hw_IndicationHeaderUnion_indHeaderF1);
  hw_IndicationHeaderFormat1_table_t imf1 = u.value;
  return hw_IndicationHeaderFormat1_param(imf1);
}

/* Call Process ID: Not defined for E2SM-HW */

/* E2SM Control message */
byte_array_t e2sm_hw_fb_generate_control_message(byte_array_t message)
{
  assert(message.buf && message.len > 0);

  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  hw_ControlMessageFormat1_start(B);
  hw_ControlMessageFormat1_param_create(B, message.buf, message.len);
  hw_ControlMessageFormat1_ref_t ctrl = hw_ControlMessageFormat1_end(B);
  hw_ControlMessageUnion_union_ref_t u = hw_ControlMessageUnion_as_ctrlMessageF1(ctrl);
  hw_ControlMessage_create_as_root(B, u);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

/* E2SM Control header */
byte_array_t e2sm_hw_fb_generate_control_header(uint32_t c)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  hw_ControlHeaderFormat1_ref_t ctrl = hw_ControlHeaderFormat1_create(B, c);
  hw_ControlHeaderUnion_union_ref_t u = hw_ControlHeaderUnion_as_ctrlHeaderF1(ctrl);
  hw_ControlHeader_create_as_root(B, u);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}
