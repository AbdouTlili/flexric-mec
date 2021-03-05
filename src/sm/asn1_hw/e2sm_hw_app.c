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

#include "e2sm_hw_app.h"

#include "E2SM-HelloWorld-ActionDefinition.h"
#include "E2SM-HelloWorld-ActionDefinition-Format1.h"
#include "E2SM-HelloWorld-EventTriggerDefinition.h"
#include "E2SM-HelloWorld-EventTriggerDefinition-Format1.h"
#include "E2SM-HelloWorld-IndicationMessage.h"
#include "E2SM-HelloWorld-IndicationMessage-Format1.h"
#include "E2SM-HelloWorld-IndicationHeader.h"
#include "E2SM-HelloWorld-IndicationHeader-Format1.h"
#include "E2SM-HelloWorld-ControlMessage.h"
#include "E2SM-HelloWorld-ControlMessage-Format1.h"
#include "E2SM-HelloWorld-ControlHeader.h"
#include "E2SM-HelloWorld-ControlHeader-Format1.h"
#include "RANparameter-List.h"
#include "RANparameter-Item.h"

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
  e2sm_hw_app_t* app = (e2sm_hw_app_t*) data;

  struct timeval now;
  gettimeofday(&now, NULL);
  const uint32_t usec_rtt = get_usec_diff(app->control_sent, now);

  const long h = e2sm_hw_decode_indication_header(i->hdr);
  if (h == app->count) {
    printf("[E2SM_HW_APP] received expected indication no %ld after %d usec\n", h, usec_rtt);
    fflush(stdout);
  } else {
    printf("[E2SM_HW_APP] received indication no %ld after %d usec, but expected no %d\n",
           h, usec_rtt, app->count);
    assert(0 && "unexpected indication number");
  }
}

static
void send_control(e2ap_ric_t* ric, int tfd, void* data)
{
  assert(ric);
  assert(data);
  e2sm_hw_app_t* app = (e2sm_hw_app_t*) data;
  assert(app->fd == tfd);

  app->count = 0;

  gettimeofday(&app->control_sent, NULL);
  app->count += 1;

  char *ctrl_text = "Hello from E2SM-HW, this is a control";
  byte_array_t ctrl_msg = { .buf = (uint8_t*) ctrl_text, .len = strlen(ctrl_text) };
  byte_array_t msg = e2sm_hw_generate_control_message(ctrl_msg);
  byte_array_t hdr = e2sm_hw_generate_control_header(app->count);
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
  e2sm_hw_app_t* app = (e2sm_hw_app_t*) data;

  printf("[E2SM_HW_APP] received subscription response, start RTT ping thread\n");

  int ms = app->ping_interval_ms;
  app->fd = e2ap_add_timer_epoll_ms_server(app->server, ms, ms, send_control, app);

  if (app->fd >= 0) {
    printf("[E2SM_HW_APP] created event to send control every %d ms\n", app->ping_interval_ms);
  } else {
    printf("[E2SM_HW_APP] could not create event\n");
    app->ran = NULL;
    app->assoc_id = -1;
  }
}

static
void sub_fail_cb(const ric_subscription_failure_t* r, void *data)
{
  assert(r && data);
  e2sm_hw_app_t* app = (e2sm_hw_app_t*) data;

  printf("[E2SM_HW_APP] received subscription failure for RAN %lx assoc ID %d, dropping it\n",
         app->ran->generated_ran_key, app->assoc_id);
  app->ran = NULL;
  app->assoc_id = -1;
}

static
void subscribe(const e2node_event_t* ev, void *data)
{
  assert(ev && data);
  e2sm_hw_app_t* app = (e2sm_hw_app_t*) data;

  if (app->ran) {
    printf("[E2SM_HW_APP] new E2 node RAN %lx assoc_id %d, but already active RAN %lx assoc_id %d\n",
           ev->ran->generated_ran_key, ev->assoc_id, app->ran->generated_ran_key, app->assoc_id);
    return;
  }

  const entity_info_t* e = ran_mgmt_get_entity(ev->ran, ev->assoc_id);
  for (size_t i = 0; i < e->e2_setup_request.len_rf; ++i) {
    const ran_function_t* rf = &e->e2_setup_request.ran_func_item[i];
    if (strncmp((char*) rf->def.buf, "This is the E2SM-HW", rf->def.len) != 0)
      continue;

    printf("[E2SM_HW_APP] new E2 node RAN %lx assoc_id %d has E2SM-HW on RAN function ID %d\n",
           ev->ran->generated_ran_key, ev->assoc_id, rf->id);
    app->ran = (ran_t*) ev->ran;
    app->assoc_id = ev->assoc_id;
    app->ric_id.ran_func_id = rf->id;

    byte_array_t event = e2sm_hw_generate_event_trigger_definition(HW_TriggerNature_onchange);
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
  printf("[E2SM_HW_APP] new E2 node RAN %lx assoc_id %d, but no RF with E2SM-HW present\n",
         ev->ran->generated_ran_key, ev->assoc_id);
}

static
void sub_del_resp_cb(const ric_subscription_delete_response_t* dr, void* data)
{
  assert(dr && data);
  e2sm_hw_app_t* app = (e2sm_hw_app_t*) data;
  printf("[E2SM_HW_APP] received subscription delete response\n");
  app->ran = NULL;
  app->assoc_id = -1;
}

static
void sub_del_fail_cb(const ric_subscription_delete_failure_t* df, void *data)
{
  assert(df && data);
  e2sm_hw_app_t* app = (e2sm_hw_app_t*) data;
  printf("[E2SM_HW_APP] received subscription delete failure from assoc_id %d\n", app->assoc_id);
  assert(0 && "should not happen");
}

static
void e2sm_hw_unsubscribe(e2sm_hw_app_t* app)
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

void e2sm_hw_app_init(e2sm_hw_app_t* app, e2ap_ric_t* server, unsigned int ping_interval_ms)
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
  printf("[E2SM_HW_APP] init app done\n");
}

void e2sm_hw_app_free(e2sm_hw_app_t* app)
{
  printf("[E2SM_HW_APP] free app start\n");
  assert(app);
  if (app->fd >= 0)
    e2ap_remove_timer_epoll_server(app->fd);

  if (app->ran) {
    e2sm_hw_unsubscribe(app);
    while (app->ran)
      usleep(100000);
  }
  printf("[E2SM_HW_APP] free app done\n");
}

byte_array_t e2sm_hw_generate_action_definition(void)
{
  RANparameter_Item_t ranParamIt;
  ranParamIt.ranParameter_ID = 12;
  uint8_t *buf = (uint8_t *)"1337";
  ranParamIt.ranParameter_Name.buf = buf;
  ranParamIt.ranParameter_Name.size = 4;
  ranParamIt.ranParameter_Test = RANparameter_Test_contains;
  /* the "designer" of the E2SM-HW was out of creativity, and used the
   * OCTET_STRING_t again :( */
  ranParamIt.ranParameter_Value.buf = buf;
  ranParamIt.ranParameter_Value.size = 4;

  RANparameter_List_t ranParameter_List;
  const int n_items = 1;
  RANparameter_Item_t items[n_items];
  ranParameter_List.list.array = (RANparameter_Item_t **) &items;
  ranParameter_List.list.count = 0;
  ranParameter_List.list.size = n_items;

  E2SM_HelloWorld_ActionDefinition_Format1_t adf1;
  adf1.ranParameter_List = ranParameter_List;
  ASN_SEQUENCE_ADD(&adf1.ranParameter_List.list, &ranParamIt);

  E2SM_HelloWorld_ActionDefinition_t ad;
  ad.present = E2SM_HelloWorld_ActionDefinition_PR_actionDefinition_Format1;
  ad.choice.actionDefinition_Format1 = &adf1;

  char error_buf[300];
  size_t errlen = 0;
  int rc = asn_check_constraints(&asn_DEF_E2SM_HelloWorld_ActionDefinition, &ad, error_buf, &errlen);
  assert(rc != -1);

  byte_array_t b = { .buf = malloc(128), .len = 128 };
  assert(b.buf);
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_enc_rval_t er = asn_encode_to_buffer(NULL, syntax, &asn_DEF_E2SM_HelloWorld_ActionDefinition, &ad, b.buf, b.len);
  assert(er.encoded <= (ssize_t) b.len);
  assert(er.encoded > 0);
  b.len = er.encoded;

  //ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_HelloWorld_ActionDefinition, &ad);
  return b;
}

/* E2SM-HW Event trigger */
byte_array_t e2sm_hw_generate_event_trigger_definition(e_HW_TriggerNature n)
{
  E2SM_HelloWorld_EventTriggerDefinition_Format1_t edf1;
  edf1.triggerNature = n;

  E2SM_HelloWorld_EventTriggerDefinition_t etd;
  etd.present = E2SM_HelloWorld_EventTriggerDefinition_PR_eventDefinition_Format1;
  etd.choice.eventDefinition_Format1 = &edf1;

  char error_buf[300];
  size_t errlen = 0;
  int rc = asn_check_constraints(&asn_DEF_E2SM_HelloWorld_EventTriggerDefinition, &etd, error_buf, &errlen);
  assert(rc != -1);

  byte_array_t b = { .buf = malloc(128), .len = 128 };
  assert(b.buf);
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_enc_rval_t er = asn_encode_to_buffer(NULL, syntax, &asn_DEF_E2SM_HelloWorld_EventTriggerDefinition, &etd, b.buf, b.len);
  assert(er.encoded <= (ssize_t) b.len);
  assert(er.encoded > 0);
  b.len = er.encoded;

  /* No need for ASN_STRUCT_FREE_CONTENTS_ONLY(), no heap allocation! */

  return b;
}

byte_array_t e2sm_hw_decode_indication_message(byte_array_t b)
{
  E2SM_HelloWorld_IndicationMessage_t *im = calloc(1, sizeof(*im));
  assert(im);
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_dec_rval_t rval = asn_decode(NULL, syntax, &asn_DEF_E2SM_HelloWorld_IndicationMessage, (void **) &im, b.buf, b.len);
  assert(rval.code == RC_OK);

  //printf("\nreceived E2SM_HelloWorld_IndicationMessage_t:\n");
  //xer_fprint(stdout, &asn_DEF_E2SM_HelloWorld_IndicationMessage, im);

  assert(im->present == E2SM_HelloWorld_IndicationMessage_PR_indicationMessage_Format1);
  assert(im->choice.indicationMessage_Format1);
  assert(im->choice.indicationMessage_Format1->indicationMsgParam.buf);
  const byte_array_t ind = {
    .buf = im->choice.indicationMessage_Format1->indicationMsgParam.buf,
    .len = im->choice.indicationMessage_Format1->indicationMsgParam.size
  };
  im->choice.indicationMessage_Format1->indicationMsgParam.buf = NULL;

  ASN_STRUCT_FREE(asn_DEF_E2SM_HelloWorld_IndicationMessage, im);
  return ind;
}

long e2sm_hw_decode_indication_header(byte_array_t b)
{
  E2SM_HelloWorld_IndicationHeader_t *ih = calloc(1, sizeof(*ih));
  assert(ih);
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_dec_rval_t rval = asn_decode(NULL, syntax, &asn_DEF_E2SM_HelloWorld_IndicationHeader, (void **) &ih, b.buf, b.len);
  assert(rval.code == RC_OK);

  //printf("\nreceived E2SM_HelloWorld_IndicationHeader:\n");
  //xer_fprint(stdout, &asn_DEF_E2SM_HelloWorld_IndicationHeader, ih);

  assert(ih->present == E2SM_HelloWorld_IndicationHeader_PR_indicationHeader_Format1);
  assert(ih->choice.indicationHeader_Format1);
  const long h = ih->choice.indicationHeader_Format1->indicationHeaderParam;

  ASN_STRUCT_FREE(asn_DEF_E2SM_HelloWorld_IndicationHeader, ih);
  return h;
}

/* Call Process ID: Not defined for E2SM-HW */

/* E2SM Control message */
byte_array_t e2sm_hw_generate_control_message(byte_array_t ctrl)
{
  E2SM_HelloWorld_ControlMessage_Format1_t cmf1;
  cmf1.controlMsgParam.buf = ctrl.buf;
  cmf1.controlMsgParam.size = ctrl.len;

  E2SM_HelloWorld_ControlMessage_t cm;
  cm.present = E2SM_HelloWorld_ControlMessage_PR_controlMessage_Format1;
  cm.choice.controlMessage_Format1 = &cmf1;

  char error_buf[300];
  size_t errlen = 0;
  int rc = asn_check_constraints(&asn_DEF_E2SM_HelloWorld_ControlMessage, &cm, error_buf, &errlen);
  assert(rc != -1);

  byte_array_t b = { .buf = malloc(ctrl.len + 30), .len = ctrl.len + 30 };
  assert(b.buf);
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_enc_rval_t er = asn_encode_to_buffer(NULL, syntax, &asn_DEF_E2SM_HelloWorld_ControlMessage, &cm, b.buf, b.len);
  assert(er.encoded <= (ssize_t) b.len);
  assert(er.encoded > 0);
  b.len = er.encoded;

  /* No need for ASN_STRUCT_FREE_CONTENTS_ONLY(), no heap allocation! */
  return b;
}

/* E2SM Control header */
byte_array_t e2sm_hw_generate_control_header(long c)
{
  E2SM_HelloWorld_ControlHeader_Format1_t chf1;
  chf1.controlHeaderParam = c;

  E2SM_HelloWorld_ControlHeader_t ch;
  ch.present = E2SM_HelloWorld_ControlHeader_PR_controlHeader_Format1;
  ch.choice.controlHeader_Format1 = &chf1;

  char error_buf[300];
  size_t errlen = 0;
  int rc = asn_check_constraints(&asn_DEF_E2SM_HelloWorld_ControlHeader, &ch, error_buf, &errlen);
  assert(rc != -1);

  byte_array_t b = { .buf = malloc(128), .len = 128 };
  assert(b.buf);
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_enc_rval_t er = asn_encode_to_buffer(NULL, syntax, &asn_DEF_E2SM_HelloWorld_ControlHeader, &ch, b.buf, b.len);
  assert(er.encoded <= (ssize_t) b.len);
  assert(er.encoded > 0);
  b.len = er.encoded;

  /* No need for ASN_STRUCT_FREE_CONTENTS_ONLY(), no heap allocation! */
  return b;
}

/* RIC Control Outcome: Not defined for E2SM-HW */
