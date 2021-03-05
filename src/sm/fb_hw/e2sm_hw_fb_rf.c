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

#include <stdio.h>
#include <assert.h>

#include "e2sm_hw_fb_rf.h"
#include "e2sm_hw_builder.h"
#include "e2sm_hw_verifier.h"
#include "e2sm_hw_json_printer.h"

static
void e2sm_hw_fb_handle_timer(e2ap_agent_t* ag, int tfd, void* data)
{
  assert(ag);
  assert(!data);
  assert(tfd > -1);

  printf("[E2SM-HW-FB] sending indication message after trigger now\n");
  byte_array_t ind_hdr = e2sm_hw_fb_generate_indication_header(111);
  const char *text = "Hello from E2SM-HW-FB, this is an indication after trigger now";
  byte_array_t ind_text = { .buf = (uint8_t*) text, .len = strlen(text) };
  byte_array_t ind_msg = e2sm_hw_fb_generate_indication_message(ind_text);

  subscription_t* sub = (subscription_t*) data;

  const ric_indication_t ind = {
    .ric_id = sub->req->ric_id,
    .action_id = sub->req->len_action > 0 ? sub->req->action[0].id : 123,
    .sn = NULL,
    .type = sub->req->len_action > 0 ? sub->req->action[0].type : RIC_IND_REPORT,
    .hdr = ind_hdr,
    .msg = ind_msg,
    .call_process_id = NULL,
  };
  e2ap_send_indication_agent(ag, sub->ep_id, &ind);
  free_byte_array(ind_hdr);
  free_byte_array(ind_msg);

  e2ap_remove_timer_epoll_agent(tfd);
}

static
bool e2sm_hw_fb_handle_subscription_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req);
  assert(!rfdata);
  const ric_subscription_request_t* sr = sub->req;

  printf("[E2SM-HW-FB] received subscription request from ep_id %d requestor %d instance %d\n",
         sub->ep_id, sr->ric_id.ric_req_id, sr->ric_id.ric_inst_id);

  /* list all actions */
  ric_action_admitted_t admitted[sr->len_action];
  for (size_t i = 0; i < sr->len_action; ++i) {
    admitted[i].ric_act_id = sr->action[i].id;
    printf("[E2SM-HW-FB] action no. %ld: ID %d type %d\n", i, sr->action[i].id, sr->action[i].type);
    if (sr->action[i].definition)
      e2sm_hw_fb_decode_action_definition(*sr->action[i].definition);
    if (sr->action[i].subseq_action)
      printf("[E2SM-HW-FB] ignoring subsequent action!\n");
  }

  const hw_TriggerNature_enum_t tn = e2sm_hw_fb_decode_event_trigger_definition(sr->event_trigger);
  if (tn == hw_TriggerNature_Now) {
    /* setup timer */
    e2ap_add_timer_epoll_ms_agent(ag, 5, 1000, e2sm_hw_fb_handle_timer, sub);
    printf("[E2SM-HW-FB] trigger nature now: sending indication message in 5ms\n");
  } else { /* HW_TriggerNature_onchange */
    printf("[E2SM-HW-FB] trigger nature onchange -> send indication as response to control\n");
  }

  ric_subscription_response_t resp = {
    .ric_id = sr->ric_id,
    .admitted = admitted,
    .len_admitted = sr->len_action
  };
  e2ap_send_subscription_response(ag, sub->ep_id, &resp);
  return true;
}

static
bool e2sm_hw_fb_handle_subscription_delete_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->data);
  assert(!rfdata);

  printf("[E2SM-HW-FB] received subscription delete from ep_id %d\n", sub->ep_id);

  ric_subscription_delete_response_t sub_del_resp = { .ric_id = sub->req->ric_id };
  e2ap_send_subscription_delete_response(ag, sub->ep_id, &sub_del_resp);
  return true;
}

static
void e2sm_hw_fb_handle_control_request(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_request_t* cr, void* rfdata)
{
  assert(ag);
  assert(cr);
  assert(!cr->call_process_id);
  assert(!cr->ack_req);
  assert(!rfdata);

  const long hdr = e2sm_hw_fb_decode_control_header(cr->hdr);
  //printf("[E2SM-HW-FB] control request header : %ld\n", hdr);
  byte_array_t msg = e2sm_hw_fb_decode_control_message(cr->msg);
  //printf("[E2SM-HW-FB] control request message: %.*s\n", (int)msg.len, (char*)msg.buf);

  subscription_t* sub = e2ap_find_subscription(ag, ep_id, cr->ric_id);
  if (!sub) {
    printf("[E2SM-HW-FB] did not receive subscription from ep_id %d ric_id %d/%d/%d, not sending indication\n",
           ep_id, cr->ric_id.ric_req_id, cr->ric_id.ric_inst_id, cr->ric_id.ran_func_id);
    return;
  }

  static int count = 1;
  printf("[E2SM-HW-FB] sending indication message after control request (no %d) to ep_id %d\n",
         count++, ep_id);
  byte_array_t ind_hdr = e2sm_hw_fb_generate_indication_header(hdr);
  byte_array_t ind_msg = e2sm_hw_fb_generate_indication_message(msg);
  free_byte_array(msg);
  //printf("hdr len %ld msg len %ld\n", ind_hdr.len, ind_msg.len);

  const ric_indication_t ind = {
    .ric_id = cr->ric_id,
    .action_id = sub->req->len_action > 0 ? sub->req->action[0].id : 123,
    .sn = NULL,
    .type = sub->req->len_action > 0 ? sub->req->action[0].type : RIC_IND_REPORT,
    .hdr = ind_hdr,
    .msg = ind_msg,
    .call_process_id = NULL,
  };
  e2ap_send_indication_agent(ag, ep_id, &ind);
  free_byte_array(ind_hdr);
  free_byte_array(ind_msg);
}

void e2sm_hw_fb_register_ran_function(e2ap_agent_t* ag)
{
  const char *def = "This is the E2SM-HW-FB";
  byte_array_t definition = { .buf = (uint8_t*)def, .len = strlen(def) };
  const ran_function_t r = {
    .def = definition,
    .id = 1,
    .rev = 0,
    .oid = NULL
  };
  service_model_cb_t cbs = {
    .handle_subscription_request = e2sm_hw_fb_handle_subscription_request,
    .handle_subscription_delete_request = e2sm_hw_fb_handle_subscription_delete_request,
    .handle_control_request = e2sm_hw_fb_handle_control_request,
  };
  e2ap_register_ran_fun(ag, &r, cbs, NULL);
}

void e2sm_hw_fb_decode_action_definition(byte_array_t b)
{
  assert(b.buf && b.len > 0);

  int ret;
  if ((ret = hw_ActionDefinition_verify_as_root(b.buf, b.len))) {
    printf("ActionDefinition is invalid: %s\n", flatcc_verify_error_string(ret));
    assert(0);
  }

  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init(&printer_ctx, stdout);
  printer_ctx.indent = 0;
  hw_ActionDefinition_print_json_as_root(&printer_ctx, b.buf, b.len, NULL);
  flatcc_json_printer_finalize(&printer_ctx);
  flatcc_json_printer_flush(&printer_ctx);
  flatcc_json_printer_clear(&printer_ctx);

  /*
  hw_ActionDefinition_table_t ad = hw_ActionDefinition_as_root(b.buf);
  hw_ActionDefinitionUnion_union_t u = hw_ActionDefinition_def_union(ad);
  assert(u.type == hw_ActionDefinitionUnion_actionDefF1);
  hw_ActionDefinitionFormat1_table_t adf1 = u.value;
  hw_RanParameterItem_vec_t list = hw_ActionDefinitionFormat1_ranParameterList(adf1);
  printf("ActionDefinition content:\n");
  for (size_t i = 0; i < hw_RanParameterItem_vec_len(list); ++i) {
    hw_RanParameterItem_table_t t = hw_RanParameterItem_vec_at(list, i);
    printf("  RanParameterItem %ld ID %d name %s test %s value %s\n",
           i, hw_RanParameterItem_id(t), hw_RanParameterItem_name(t),
           hw_RanParameterTest_name(hw_RanParameterItem_test(t)),
           hw_RanParameterItem_value(t));
  }
  */
  return;
}

hw_TriggerNature_enum_t e2sm_hw_fb_decode_event_trigger_definition(byte_array_t b)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = hw_EventTrigger_verify_as_root(b.buf, b.len))) {
    printf("EventTrigger is invalid: %s\n", flatcc_verify_error_string(ret));
    assert(0);
  }
  hw_EventTrigger_table_t et = hw_EventTrigger_as_root(b.buf);
  hw_EventTriggerUnion_union_t u = hw_EventTrigger_trig_union(et);
  assert(u.type == hw_EventTriggerUnion_eventDefF1);
  hw_EventTriggerDefinitionFormat1_table_t etdf1 = u.value;
  return hw_EventTriggerDefinitionFormat1_triggerNature(etdf1);
}

/* E2SM Indication message (Report, Insert) */
byte_array_t e2sm_hw_fb_generate_indication_message(byte_array_t message)
{
  assert(message.buf && message.len > 0);

  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  hw_IndicationMessageFormat1_start(B);
  hw_IndicationMessageFormat1_param_create(B, message.buf, message.len);
  hw_IndicationMessageFormat1_ref_t ind = hw_IndicationMessageFormat1_end(B);
  hw_IndicationMessageUnion_union_ref_t u = hw_IndicationMessageUnion_as_indMessageF1(ind);
  hw_IndicationMessage_create_as_root(B, u);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

/* E2SM Indication Header */
byte_array_t e2sm_hw_fb_generate_indication_header(uint32_t header)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  hw_IndicationHeaderFormat1_ref_t ind = hw_IndicationHeaderFormat1_create(B, header);
  hw_IndicationHeaderUnion_union_ref_t u = hw_IndicationHeaderUnion_as_indHeaderF1(ind);
  hw_IndicationHeader_create_as_root(B, u);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

/* Call Process ID: Not defined for E2SM-HW */

byte_array_t e2sm_hw_fb_decode_control_message(byte_array_t b)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = hw_ControlMessage_verify_as_root(b.buf, b.len))) {
    printf("EventTrigger is invalid: %s\n", flatcc_verify_error_string(ret));
    assert(0);
  }
  hw_ControlMessage_table_t cm = hw_ControlMessage_as_root(b.buf);
  hw_ControlMessageUnion_union_t u = hw_ControlMessage_msg_union(cm);
  assert(u.type == hw_ControlMessageUnion_ctrlMessageF1);
  hw_ControlMessageFormat1_table_t cmf1 = u.value;
  const uint8_t* p = hw_ControlMessageFormat1_param(cmf1);
  const size_t pl = flatbuffers_uint8_vec_len(p);
  byte_array_t bret = { .buf = malloc(pl), .len = pl };
  assert(bret.buf);
  memcpy(bret.buf, p, pl);
  return bret;
}

uint32_t e2sm_hw_fb_decode_control_header(byte_array_t b)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = hw_ControlHeader_verify_as_root(b.buf, b.len))) {
    printf("EventTrigger is invalid: %s\n", flatcc_verify_error_string(ret));
    assert(0);
  }
  hw_ControlHeader_table_t ch = hw_ControlHeader_as_root(b.buf);
  hw_ControlHeaderUnion_union_t u = hw_ControlHeader_hdr_union(ch);
  assert(u.type == hw_ControlHeaderUnion_ctrlHeaderF1);
  hw_ControlHeaderFormat1_table_t chf1 = u.value;
  return hw_ControlHeaderFormat1_param(chf1);
}

/* RIC Control Outcome: Not defined for E2SM-HW */
