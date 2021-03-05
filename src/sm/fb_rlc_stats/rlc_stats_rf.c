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

#include "rlc_stats_rf.h"
#include "flexric_agent.h"
#include "rlc_stats_msg_enc.h"
#include "rlc_stats_msg_dec.h"

rlc_stats_callbacks_t g_rlc_stats_cb;

typedef struct rlc_stats_timer_data_s {
  rlc_stats_report_style_t report_style;
  const ric_subscription_request_t* sr;
  int tfd;
} rlc_stats_timer_data_t;

static
void rlc_stats_handle_timer(e2ap_agent_t* ag, int tfd, void* data)
{
  assert(ag);
  assert(data);
  assert(tfd > -1);

  const rlc_stats_timer_data_t* t = (rlc_stats_timer_data_t*)data;
  assert(t->tfd == tfd);

  byte_array_t ind_hdr = rlc_stats_encode_indication_header(g_rlc_stats_cb.hdr);
  byte_array_t ind_msg = rlc_stats_encode_indication_message(g_rlc_stats_cb.msg, &t->report_style);

  const ric_indication_t ind = {
    .ric_id = t->sr->ric_id,
    .action_id = t->sr->action[0].id,
    .type = RIC_IND_REPORT,
    .hdr = ind_hdr,
    .msg = ind_msg,
  };
  e2ap_send_indication_agent(ag, 0, &ind);
  free_byte_array(ind_hdr);
  free_byte_array(ind_msg);
}

static
bool rlc_stats_handle_subscription_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->ep_id == 0);
  assert(!rfdata);
  const ric_subscription_request_t* sr = sub->req;

  const uint16_t trigger_ms = rlc_stats_decode_event_trigger(sr->event_trigger);

  assert(sr->len_action == 1 && "cannot handle more than one action");
  assert(sr->action[0].type == RIC_ACT_REPORT);
  rlc_stats_timer_data_t* t = malloc(sizeof(*t));
  t->report_style = sr->action[0].definition
          ? rlc_stats_decode_action_definition(*sr->action[0].definition)
          : (rlc_stats_report_style_t) { .type = RLC_STATS_REPORT_STYLE_TYPE_MINIMAL };
  t->sr = sr;

  t->tfd = e2ap_add_timer_epoll_ms_agent(ag, trigger_ms, trigger_ms, rlc_stats_handle_timer, t);

  ric_action_admitted_t admitted = { .ric_act_id = sr->action[0].id };
  ric_subscription_response_t resp = {
    .ric_id = sr->ric_id,
    .admitted = &admitted,
    .len_admitted = 1
  };
  e2ap_send_subscription_response(ag, 0, &resp);
  return true;
}

static
bool rlc_stats_handle_subscription_delete_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->ep_id == 0 && sub->data);
  assert(!rfdata);
  rlc_stats_timer_data_t* t = sub->data;
  e2ap_remove_timer_epoll_agent(t->tfd);
  free(sub->data);
  ric_subscription_delete_response_t resp = { .ric_id = sub->req->ric_id };
  e2ap_send_subscription_delete_response(ag, 0, &resp);
  return true;
}

static
void rlc_stats_handle_control_request(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_request_t* cr, void* rfdata)
{
  assert(ag);
  assert(ep_id == 0);
  assert(cr);
  assert(!rfdata);
  assert(0 && "there is no control for the rlc_stats sm");
}

void sm_rlc_stats_register_ran_function(e2ap_agent_t* ag, rlc_stats_callbacks_t stats_cb, const rlc_stats_report_style_t* styles, size_t n_styles)
{
  assert(stats_cb.hdr && stats_cb.msg);
  assert(styles && n_styles > 0);

  g_rlc_stats_cb = stats_cb;

  service_model_cb_t cbs = {
    .handle_subscription_request = rlc_stats_handle_subscription_request,
    .handle_subscription_delete_request = rlc_stats_handle_subscription_delete_request,
    .handle_control_request = rlc_stats_handle_control_request,
  };

  byte_array_t oid = { .buf = (uint8_t*) rlc_stats_oid, .len = strlen(rlc_stats_oid) };
  const ran_function_t r = {
    .def = rlc_stats_encode_ran_function(styles, n_styles),
    .id = RLC_STATS_RF,
    .rev = 0,
    .oid = &oid
  };

  e2ap_register_ran_fun(ag, &r, cbs, NULL);

  free_byte_array(r.def);
}
