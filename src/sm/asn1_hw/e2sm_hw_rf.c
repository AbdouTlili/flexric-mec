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

#include "e2sm_hw_rf.h"

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

void e2sm_hw_handle_timer(e2ap_agent_t* ag, int tfd, void* data)
{
  assert(ag);
  assert(data);
  assert(tfd > -1);

  printf("[E2SM-HW] sending indication message after trigger now\n");
  byte_array_t ind_hdr = e2sm_hw_generate_indication_header(111);
  const char *text = "Hello from E2SM-HW, this is an indication after trigger now";
  byte_array_t ind_text = { .buf = (uint8_t*) text, .len = strlen(text) };
  byte_array_t ind_msg = e2sm_hw_generate_indication_message(ind_text);

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

bool e2sm_hw_handle_subscription_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req);
  assert(!rfdata);
  const ric_subscription_request_t* sr = sub->req;

  printf("[E2SM-HW] received subscription request from ep_id %d requestor %d instance %d\n",
         sub->ep_id, sr->ric_id.ric_req_id, sr->ric_id.ric_inst_id);

  /* list all actions */
  ric_action_admitted_t admitted[sr->len_action];
  for (size_t i = 0; i < sr->len_action; ++i) {
    admitted[i].ric_act_id = sr->action[i].id;
    printf("[E2SM-HW] action no. %ld: ID %d type %d\n", i, sr->action[i].id, sr->action[i].type);
    if (sr->action[i].definition)
      e2sm_hw_decode_action_definition(*sr->action[i].definition);
    if (sr->action[i].subseq_action)
      printf("[E2SM-HW] ignoring subsequent action!\n");
  }

  const e_HW_TriggerNature tn = e2sm_hw_decode_event_trigger_definition(sr->event_trigger);
  if (tn == HW_TriggerNature_now) {
    /* setup timer */
    e2ap_add_timer_epoll_ms_agent(ag, 5, 1000, e2sm_hw_handle_timer, sub);
    printf("[E2SM-HW] trigger nature now: sending indication message in 5ms\n");
  } else { /* HW_TriggerNature_onchange */
    printf("[E2SM-HW] trigger nature onchange -> send indication as response to control\n");
  }

  ric_subscription_response_t resp = {
    .ric_id = sr->ric_id,
    .admitted = admitted,
    .len_admitted = sr->len_action
  };
  e2ap_send_subscription_response(ag, sub->ep_id, &resp);
  return true;
}

bool e2sm_hw_handle_subscription_delete_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->data);
  assert(!rfdata);

  printf("[E2SM-HW] received subscription delete\n");

  ric_subscription_delete_response_t sub_del_resp = { .ric_id = sub->req->ric_id };
  e2ap_send_subscription_delete_response(ag, sub->ep_id, &sub_del_resp);
  return true;
}

void e2sm_hw_handle_control_request(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_request_t* cr, void* rfdata)
{
  assert(ag);
  assert(cr);
  assert(!cr->call_process_id);
  assert(!cr->ack_req);
  assert(!rfdata);

  const long hdr = e2sm_hw_decode_control_header(cr->hdr);
  //printf("[E2SM-HW] control request header : %ld\n", hdr);
  byte_array_t msg = e2sm_hw_decode_control_message(cr->msg);
  //printf("[E2SM-HW] control request message: %.*s\n", (int)msg.len, (char*)msg.buf);

  subscription_t* sub = e2ap_find_subscription(ag, ep_id, cr->ric_id);
  if (!sub) {
    printf("[E2SM-HW] did not receive subscription from ep_id %d ric_id %d/%d/%d, not sending indication\n",
           ep_id, cr->ric_id.ric_req_id, cr->ric_id.ric_inst_id, cr->ric_id.ran_func_id);
    return;
  }

  static int count = 1;
  printf("[E2SM-HW] sending indication message after control request (no %d) to ep_id %d\n",
         count++, ep_id);
  byte_array_t ind_hdr = e2sm_hw_generate_indication_header(hdr);
  byte_array_t ind_msg = e2sm_hw_generate_indication_message(msg);
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

void e2sm_hw_register_ran_function(e2ap_agent_t* ag)
{
  service_model_cb_t cbs = {
    .handle_subscription_request = e2sm_hw_handle_subscription_request,
    .handle_subscription_delete_request = e2sm_hw_handle_subscription_delete_request,
    .handle_control_request = e2sm_hw_handle_control_request,
  };
  const char *def = "This is the E2SM-HW";
  const size_t len = strlen(def);
  byte_array_t definition = { .buf = (uint8_t*)def, .len = len };
  ran_function_t r = {
    .def = definition,
    .id = 1,
    .rev = 0,
    .oid = NULL
  };
  e2ap_register_ran_fun(ag, &r, cbs, NULL);
}

void e2sm_hw_decode_action_definition(byte_array_t b)
{
  E2SM_HelloWorld_ActionDefinition_t *ad = calloc(1, sizeof(*ad));
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_dec_rval_t rval = asn_decode(NULL, syntax, &asn_DEF_E2SM_HelloWorld_ActionDefinition, (void **) &ad, b.buf, b.len);
  assert(rval.code == RC_OK);

  //printf("\nreceived E2SM_HelloWorld_ActionDefinition_Format:");
  //for (size_t i = 0; i < b.len; ++i)
  //  printf(" %02x", b.buf[i]);
  //printf("\n");
  //xer_fprint(stdout, &asn_DEF_E2SM_HelloWorld_ActionDefinition, ad);

  ASN_STRUCT_FREE(asn_DEF_E2SM_HelloWorld_ActionDefinition, ad);
}

e_HW_TriggerNature e2sm_hw_decode_event_trigger_definition(byte_array_t b)
{
  E2SM_HelloWorld_EventTriggerDefinition_t *edf = calloc(1, sizeof(*edf));
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_dec_rval_t rval = asn_decode(NULL, syntax, &asn_DEF_E2SM_HelloWorld_EventTriggerDefinition, (void **) &edf, b.buf, b.len);
  assert(rval.code == RC_OK);

  //printf("\nreceived E2SM_HelloWorld_EventTriggerDefinition:\n");
  //xer_fprint(stdout, &asn_DEF_E2SM_HelloWorld_EventTriggerDefinition, edf);

  assert(edf->present == E2SM_HelloWorld_EventTriggerDefinition_PR_eventDefinition_Format1);
  assert(edf->choice.eventDefinition_Format1);
  const e_HW_TriggerNature n = edf->choice.eventDefinition_Format1->triggerNature;

  ASN_STRUCT_FREE(asn_DEF_E2SM_HelloWorld_EventTriggerDefinition, edf);
  return n;
}

/* E2SM Indication message (Report, Insert) */
byte_array_t e2sm_hw_generate_indication_message(byte_array_t message)
{
  E2SM_HelloWorld_IndicationMessage_Format1_t imf1;
  imf1.indicationMsgParam.buf = message.buf;
  imf1.indicationMsgParam.size = message.len;

  E2SM_HelloWorld_IndicationMessage_t im;
  im.present = E2SM_HelloWorld_IndicationMessage_PR_indicationMessage_Format1;
  im.choice.indicationMessage_Format1 = &imf1;

  char error_buf[300];
  size_t errlen = 0;
  int rc = asn_check_constraints(&asn_DEF_E2SM_HelloWorld_IndicationMessage, &im, error_buf, &errlen);
  assert(rc != -1);

  byte_array_t b = { .buf = malloc(message.len+30), .len = message.len+30 };
  assert(b.buf);
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_enc_rval_t er = asn_encode_to_buffer(NULL, syntax, &asn_DEF_E2SM_HelloWorld_IndicationMessage, &im, b.buf, b.len);
  assert(er.encoded <= (ssize_t) b.len);
  assert(er.encoded > 0);
  b.len = er.encoded;

  /* No need for ASN_STRUCT_FREE_CONTENTS_ONLY(), no heap allocation! */
  return b;
}

/* E2SM Indication Header */
byte_array_t e2sm_hw_generate_indication_header(long header)
{
  E2SM_HelloWorld_IndicationHeader_Format1_t ihf1;
  ihf1.indicationHeaderParam = header;

  E2SM_HelloWorld_IndicationHeader_t ih;
  ih.present = E2SM_HelloWorld_IndicationHeader_PR_indicationHeader_Format1;
  ih.choice.indicationHeader_Format1 = &ihf1;

  char error_buf[300];
  size_t errlen = 0;
  int rc = asn_check_constraints(&asn_DEF_E2SM_HelloWorld_IndicationHeader, &ih, error_buf, &errlen);
  assert(rc != -1);

  byte_array_t b = { .buf = malloc(128), .len = 128 };
  assert(b.buf);
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_enc_rval_t er = asn_encode_to_buffer(NULL, syntax, &asn_DEF_E2SM_HelloWorld_IndicationHeader, &ih, b.buf, b.len);
  assert(er.encoded <= (ssize_t) b.len);
  assert(er.encoded > 0);
  b.len = er.encoded;

  /* No need for ASN_STRUCT_FREE_CONTENTS_ONLY(), no heap allocation! */
  return b;
}

/* Call Process ID: Not defined for E2SM-HW */

byte_array_t e2sm_hw_decode_control_message(byte_array_t b)
{
  E2SM_HelloWorld_ControlMessage_t *cm = calloc(1, sizeof(*cm));
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_dec_rval_t rval = asn_decode(NULL, syntax, &asn_DEF_E2SM_HelloWorld_ControlMessage, (void **) &cm, b.buf, b.len);
  assert(rval.code == RC_OK);

  //printf("\nreceived E2SM_HelloWorld_ControlMessage:\n");
  //xer_fprint(stdout, &asn_DEF_E2SM_HelloWorld_ControlMessage, cm);

  assert(cm->present == E2SM_HelloWorld_ControlMessage_PR_controlMessage_Format1);
  assert(cm->choice.controlMessage_Format1);
  assert(cm->choice.controlMessage_Format1->controlMsgParam.buf);
  const byte_array_t ctrl = {
    .buf = cm->choice.controlMessage_Format1->controlMsgParam.buf,
    .len = cm->choice.controlMessage_Format1->controlMsgParam.size
  };
  cm->choice.controlMessage_Format1->controlMsgParam.buf = NULL;

  ASN_STRUCT_FREE(asn_DEF_E2SM_HelloWorld_ControlMessage, cm);
  return ctrl;
}

long e2sm_hw_decode_control_header(byte_array_t b)
{
  E2SM_HelloWorld_ControlHeader_t *ch = calloc(1, sizeof(*ch));
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_dec_rval_t rval = asn_decode(NULL, syntax, &asn_DEF_E2SM_HelloWorld_ControlHeader, (void **) &ch, b.buf, b.len);
  assert(rval.code == RC_OK);

  //printf("\nreceived E2SM_HelloWorld_ControlHeader:\n");
  //xer_fprint(stdout, &asn_DEF_E2SM_HelloWorld_ControlHeader, ch);

  assert(ch->present == E2SM_HelloWorld_ControlHeader_PR_controlHeader_Format1);
  assert(ch->choice.controlHeader_Format1);
  const long c = ch->choice.controlHeader_Format1->controlHeaderParam;

  ASN_STRUCT_FREE(asn_DEF_E2SM_HelloWorld_ControlHeader, ch);
  return c;
}

/* RIC Control Outcome: Not defined for E2SM-HW */
