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
#include <stdlib.h>
#include <pthread.h>

#include "rslicing_rf.h"
#include "flexric_agent.h"
#include "rslicing_msg_enc.h"
#include "rslicing_msg_dec.h"
#include "rslicing_builder.h"

static rslicing_cb_t g_cb;

static
void rslicing_trigger_slice_info(e2ap_agent_t* ag, rslicing_fill_ind_msg_cb read_cb, ric_gen_id_t ric_id, uint8_t action_id)
{
  byte_array_t ind_msg = mac_rslicing_encode_indication_message(read_cb);
  byte_array_t ind_hdr = mac_rslicing_encode_empty_indication_header();
  const ric_indication_t ind = {
    .ric_id = ric_id,
    .action_id = action_id,
    .type = RIC_IND_REPORT,
    .hdr = ind_hdr,
    .msg = ind_msg,
  };
  e2ap_send_indication_agent(ag, 0, &ind);
  free_byte_array(ind_msg);
  free_byte_array(ind_hdr);
}

static
bool rslicing_handle_subscription_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->ep_id == 0);
  assert(!rfdata);
  const ric_subscription_request_t* sr = sub->req;
  assert(sr->len_action == 1 && "cannot handle more than one action");
  assert(sr->action[0].type == RIC_ACT_REPORT);
  assert(!sr->action[0].definition);
  assert(!sr->action[0].subseq_action);

  const mac_rslicing_ReportOccasion_enum_t occ = mac_rslicing_decode_event_trigger(sr->event_trigger);
  assert(occ == mac_rslicing_ReportOccasion_onChange);

  ric_action_admitted_t admitted = { .ric_act_id = sr->action[0].id };
  ric_subscription_response_t resp = {
    .ric_id = sr->ric_id,
    .admitted = &admitted,
    .len_admitted = 1
  };
  e2ap_send_subscription_response(ag, 0, &resp);

  rslicing_trigger_slice_info(ag, g_cb.read, sr->ric_id, sr->action[0].id);

  return true;
}

static
bool rslicing_handle_subscription_delete_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->ep_id == 0 && sub->data);
  assert(!rfdata);

  ric_subscription_delete_response_t resp = { .ric_id = sub->req->ric_id };
  e2ap_send_subscription_delete_response(ag, 0, &resp);
  return true;
}

static
void rslicing_handle_control_request(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_request_t* cr, void* rfdata)
{
  assert(ag);
  assert(ep_id == 0);
  assert(cr);
  assert(!rfdata);

  mac_rslicing_ControlMessage_table_t cm = mac_rslicing_ControlMessage_as_root(cr->msg.buf);
  mac_rslicing_ControlCommand_union_t cc = mac_rslicing_ControlMessage_command_union(cm);

  rslicing_rc_t rslicing_rc;
  switch (cc.type) {
    case mac_rslicing_ControlCommand_addModSlice:
      printf("received addModSlice ControlCommand\n");
      rslicing_rc = g_cb.add_mod(cc.value);
      break;
    case mac_rslicing_ControlCommand_delSlice:
      printf("received delSlice ControlCommand\n");
      rslicing_rc = g_cb.del(cc.value);
      break;
    case mac_rslicing_ControlCommand_ueSliceAssoc:
      printf("received ueSliceAssoc ControlCommand\n");
      rslicing_rc = g_cb.ue_assoc(cc.value);
      break;
    default:
      rslicing_rc = (rslicing_rc_t) { .success = false, .error_msg = strdup("no command") };
      break;
  }

  if (rslicing_rc.success && cr->ack_req && *cr->ack_req == RIC_CONTROL_REQUEST_ACK) {
    ric_control_acknowledge_t ca = {
      .ric_id = cr->ric_id,
      .status = RIC_CONTROL_STATUS_SUCCESS
    };
    e2ap_send_control_acknowledge(ag, 0, &ca);
  }
  if (!rslicing_rc.success && cr->ack_req
      && (*cr->ack_req == RIC_CONTROL_REQUEST_NACK || *cr->ack_req == RIC_CONTROL_REQUEST_ACK)) {
    assert(rslicing_rc.error_msg);
    byte_array_t co = mac_rslicing_encode_control_outcome(rslicing_rc.error_msg);
    ric_control_failure_t cf = {
      .ric_id = cr->ric_id,
      .cause = {
        .present = CAUSE_RICSERVICE,
        .ricService = CAUSE_RICSERVICE_RIC_RESOURCE_LIMIT
      },
      .control_outcome = &co
    };
    e2ap_send_control_failure(ag, 0, &cf);
    free_byte_array(co);
  }

  if (!rslicing_rc.success && rslicing_rc.error_msg)
    printf("error in command: %s\n", rslicing_rc.error_msg);
  if (rslicing_rc.error_msg)
    free(rslicing_rc.error_msg);

  /* send an indication if the corresponding subscription exists */
  subscription_t* sub = e2ap_find_subscription(ag, 0, cr->ric_id);
  if (sub)
    rslicing_trigger_slice_info(ag, g_cb.read, cr->ric_id, sub->req->action[0].id);
}

void sm_mac_rslicing_register_ran_function(struct e2ap_agent_s* ag, const mac_rslicing_SliceAlgorithm_enum_t* algos, size_t n, rslicing_cb_t cb)
{
  assert(ag);
  assert(algos && n > 0);
  assert(algos[0] == mac_rslicing_SliceAlgorithm_None);

  service_model_cb_t sm_cbs = {
    .handle_subscription_request = rslicing_handle_subscription_request,
    .handle_subscription_delete_request = rslicing_handle_subscription_delete_request,
    .handle_control_request = rslicing_handle_control_request,
  };

  byte_array_t oid = { .buf = (uint8_t*) rslicing_oid, .len = strlen(rslicing_oid) };
  const ran_function_t r = {
    .def = mac_rslicing_encode_ran_function(algos, n),
    .id = RSLICING_RF,
    .rev = 0,
    .oid = &oid
  };

  g_cb = cb;

  e2ap_register_ran_fun(ag, &r, sm_cbs, NULL);

  free_byte_array(r.def);
}
