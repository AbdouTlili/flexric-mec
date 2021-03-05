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

#include "rrc_conf_rf.h"
#include "flexric_agent.h"
#include "rrc_conf_msg_enc.h"
#include "rrc_conf_msg_dec.h"
#include "rrc_conf_builder.h"

rrc_conf_fill_ind_msg_cb g_cb;

static
bool rrc_conf_handle_subscription_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->ep_id == 0);
  assert(!rfdata);
  const ric_subscription_request_t* sr = sub->req;
  assert(sr->len_action == 1 && "cannot handle more than one action");
  assert(sr->action[0].type == RIC_ACT_REPORT);

  const rrc_conf_report_occasion_e occ = rrc_conf_decode_event_trigger(sr->event_trigger);
  assert(occ == RRC_CONF_REPORT_OCCASION_ONCHANGE);

  const rrc_conf_report_style_t style = sr->action[0].definition
      ? rrc_conf_decode_action_definition(*sr->action[0].definition)
      : (rrc_conf_report_style_t) { .type = RRC_CONF_REPORT_STYLE_TYPE_MINIMAL };

  ric_action_admitted_t admitted = { .ric_act_id = sr->action[0].id };
  ric_subscription_response_t resp = {
    .ric_id = sr->ric_id,
    .admitted = &admitted,
    .len_admitted = 1
  };
  e2ap_send_subscription_response(ag, 0, &resp);

  byte_array_t ind_hdr = rrc_conf_encode_indication_header();
  byte_array_t ind_msg = rrc_conf_encode_indication_message(g_cb, &style);
  const ric_indication_t ind = {
    .ric_id = sr->ric_id,
    .action_id = sr->action[0].id,
    .type = RIC_IND_REPORT,
    .hdr = ind_hdr,
    .msg = ind_msg,
  };
  e2ap_send_indication_agent(ag, 0, &ind);
  free_byte_array(ind_hdr);
  free_byte_array(ind_msg);

  return true;
}

static
bool rrc_conf_handle_subscription_delete_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->ep_id == 0 && !sub->data);
  assert(!rfdata);
  ric_subscription_delete_response_t resp = { .ric_id = sub->req->ric_id };
  e2ap_send_subscription_delete_response(ag, 0, &resp);
  return true;
}

static
void rrc_conf_handle_control_request(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_request_t* cr, void* rfdata)
{
  assert(ag);
  assert(ep_id == 0);
  assert(cr);
  assert(!rfdata);
  /* TODO: search corresponding subscription and send indication */
  assert(0 && "control response not implemented");
}

void sm_rrc_conf_register_ran_function(struct e2ap_agent_s* ag, rrc_conf_fill_ind_msg_cb cb, const rrc_conf_report_style_t* styles, size_t n_styles)
{
  assert(ag);
  assert(styles && n_styles > 0);

  service_model_cb_t cbs = {
    .handle_subscription_request = rrc_conf_handle_subscription_request,
    .handle_subscription_delete_request = rrc_conf_handle_subscription_delete_request,
    .handle_control_request = rrc_conf_handle_control_request,
  };

  byte_array_t oid = { .buf = (uint8_t*) rrc_conf_oid, .len = strlen(rrc_conf_oid) };
  const ran_function_t r = {
    .def = rrc_conf_encode_ran_function(styles, n_styles),
    .id = RRC_CONF_RF,
    .rev = 0,
    .oid = &oid
  };

  g_cb = cb;
  e2ap_register_ran_fun(ag, &r, cbs, NULL);

  free_byte_array(r.def);
}
