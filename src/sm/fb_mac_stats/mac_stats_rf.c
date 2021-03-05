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

#include "mac_stats_rf.h"
#include "flexric_agent.h"
#include "mac_stats_msg_enc.h"
#include "mac_stats_msg_dec.h"

mac_stats_callbacks_t g_mac_stats_cb;

typedef struct mac_stats_timer_data_s {
  mac_stats_report_style_t report_style;
  int tfd;
} mac_stats_timer_data_t;

static
void mac_stats_handle_timer(e2ap_agent_t* ag, int tfd, void* data)
{
  assert(ag);
  assert(data);
  assert(tfd > -1);

  subscription_t* sub = (subscription_t*) data;
  const mac_stats_timer_data_t* t = (mac_stats_timer_data_t*)sub->data;
  assert(t->tfd == tfd);

  byte_array_t ind_hdr = mac_stats_encode_indication_header(g_mac_stats_cb.hdr);
  byte_array_t ind_msg = mac_stats_encode_indication_message(g_mac_stats_cb.msg, &t->report_style);

  const ric_indication_t ind = {
    .ric_id = sub->req->ric_id,
    .action_id = sub->req->action[0].id,
    .type = RIC_IND_REPORT,
    .hdr = ind_hdr,
    .msg = ind_msg,
  };
  e2ap_send_indication_agent(ag, 0, &ind);
  free_byte_array(ind_hdr);
  free_byte_array(ind_msg);
}

static
bool mac_stats_handle_subscription_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->ep_id == 0);
  assert(!rfdata);
  const ric_subscription_request_t* sr = sub->req;

  const uint16_t trigger_ms = mac_stats_decode_event_trigger(sr->event_trigger);

  assert(sr->len_action == 1 && "cannot handle more than one action");
  assert(sr->action[0].type == RIC_ACT_REPORT);
  mac_stats_timer_data_t* t = malloc(sizeof(*t));
  t->report_style = sr->action[0].definition
          ? mac_stats_decode_action_definition(*sr->action[0].definition)
          : (mac_stats_report_style_t) { .type = MAC_STATS_REPORT_STYLE_TYPE_MINIMAL };

  t->tfd = e2ap_add_timer_epoll_ms_agent(ag, trigger_ms, trigger_ms, mac_stats_handle_timer, sub);
  sub->data = t;

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
bool mac_stats_handle_subscription_delete_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->ep_id == 0 && sub->data);
  assert(!rfdata);
  mac_stats_timer_data_t* t = sub->data;
  e2ap_remove_timer_epoll_agent(t->tfd);
  free(sub->data);
  ric_subscription_delete_response_t resp = { .ric_id = sub->req->ric_id };
  e2ap_send_subscription_delete_response(ag, 0, &resp);
  return true;
}

static
void mac_stats_handle_control_request(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_request_t* cr, void* rfdata)
{
  assert(ag);
  assert(ep_id == 0);
  assert(cr);
  assert(!rfdata);
  assert(0 && "there is no control for the mac_stats sm");
}

void sm_mac_stats_register_ran_function(e2ap_agent_t* ag, mac_stats_callbacks_t stats_cb, const mac_stats_report_style_t* styles, size_t n_styles)
{
  assert(stats_cb.hdr && stats_cb.msg);
  assert(styles && n_styles > 0);

  g_mac_stats_cb = stats_cb;

  service_model_cb_t cbs = {
    .handle_subscription_request = mac_stats_handle_subscription_request,
    .handle_subscription_delete_request = mac_stats_handle_subscription_delete_request,
    .handle_control_request = mac_stats_handle_control_request,
  };

  byte_array_t oid = { .buf = (uint8_t*) mac_stats_oid, .len = strlen(mac_stats_oid) };
  const ran_function_t r = {
    .def = mac_stats_encode_ran_function(styles, n_styles),
    .id = MAC_STATS_RF,
    .rev = 0,
    .oid = &oid
  };

  e2ap_register_ran_fun(ag, &r, cbs, NULL);

  free_byte_array(r.def);
}
