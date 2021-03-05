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

#include "rrc_stats_rf.h"
#include "flexric_agent.h"
#include "rrc_stats_msg_enc.h"
#include "rrc_stats_msg_dec.h"
#include "rrc_stats_builder.h"

typedef struct rrc_stats_sub_data_s {
  const ric_subscription_request_t* sr;
  rrc_stats_report_style_t style;
} rrc_stats_sub_data_t;

// ugly list: TODO create proper list structure
rrc_stats_sub_data_t g_subs[10];
size_t g_n_subs;
pthread_mutex_t g_subs_mtx = PTHREAD_MUTEX_INITIALIZER;

ric_subscription_failure_t get_resource_limit_failure(const ric_subscription_request_t* sr)
{
  cause_t cause = {
    .present = CAUSE_RICREQUEST,
    .ricRequest = CAUSE_RIC_FUNCTION_RESOURCE_LIMIT
  };
  ric_action_not_admitted_t not_admitted = {
    .ric_act_id = sr->action[0].id,
    .cause = cause
  };
  return (ric_subscription_failure_t) {
    .ric_id = sr->ric_id,
    .not_admitted = &not_admitted,
    .len_na = 1
  };
}

static
bool rrc_stats_handle_subscription_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->ep_id == 0);
  assert(!rfdata);
  const ric_subscription_request_t* sr = sub->req;
  assert(sr->len_action == 1 && "cannot handle more than one action");
  assert(sr->action[0].type == RIC_ACT_REPORT);

  pthread_mutex_lock(&g_subs_mtx);
  if (g_n_subs >= sizeof(g_subs) / sizeof(g_subs[0])) {
    pthread_mutex_unlock(&g_subs_mtx);
    ric_subscription_failure_t fail = get_resource_limit_failure(sr);
    e2ap_send_subscription_failure(ag, 0, &fail);
    return false;
  }

  const rrc_stats_report_occasion_e occ = rrc_stats_decode_event_trigger(sr->event_trigger);
  assert(occ == RRC_STATS_REPORT_OCCASION_ONCHANGE);

  rrc_stats_sub_data_t* new_sub = &g_subs[g_n_subs++];
  new_sub->sr = sr;
  new_sub->style = rrc_stats_decode_action_definition(*sr->action[0].definition);
  pthread_mutex_unlock(&g_subs_mtx);

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
bool rrc_stats_handle_subscription_delete_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->ep_id == 0 && sub->data);
  assert(!rfdata);
  pthread_mutex_lock(&g_subs_mtx);
  for (size_t i = 0; i < g_n_subs; ++i) {
    const ric_gen_id_t* del_id = &sub->req->ric_id;
    const ric_gen_id_t* sub_id = &g_subs[i].sr->ric_id;
    if (del_id->ric_inst_id != sub_id->ric_inst_id || del_id->ric_req_id != sub_id->ric_req_id)
      continue;

    // found the corresponding existing subscription
    g_n_subs--;
    if (i < g_n_subs)
      g_subs[i] = g_subs[g_n_subs];
    // else it was itself the last one (at all, or in the list)
    ric_subscription_delete_response_t resp = { .ric_id = sub->req->ric_id };
    e2ap_send_subscription_delete_response(ag, 0, &resp);
    pthread_mutex_unlock(&g_subs_mtx);
    return true;
  }
  pthread_mutex_unlock(&g_subs_mtx);

  cause_t cause = { .present = CAUSE_RICREQUEST, .ricRequest = CAUSE_RIC_REQUEST_ID_UNKNOWN };
  ric_subscription_delete_failure_t fail = { .ric_id = sub->req->ric_id, .cause = cause };
  e2ap_send_subscription_delete_failure(ag, 0, &fail);
  return false;
}

static
void rrc_stats_handle_control_request(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_request_t* cr, void* rfdata)
{
  assert(ag);
  assert(ep_id == 0);
  assert(cr);
  assert(!rfdata);
  assert(0 && "there is no control for the rrc_stats sm");
}

void sm_rrc_stats_register_ran_function(struct e2ap_agent_s* ag, const rrc_stats_report_style_t* styles, size_t n_styles)
{
  assert(ag);
  assert(styles && n_styles > 0);

  service_model_cb_t cbs = {
    .handle_subscription_request = rrc_stats_handle_subscription_request,
    .handle_subscription_delete_request = rrc_stats_handle_subscription_delete_request,
    .handle_control_request = rrc_stats_handle_control_request,
  };

  byte_array_t oid = { .buf = (uint8_t*) rrc_stats_oid, .len = strlen(rrc_stats_oid) };
  const ran_function_t r = {
    .def = rrc_stats_encode_ran_function(styles, n_styles),
    .id = RRC_STATS_RF,
    .rev = 0,
    .oid = &oid
  };

  e2ap_register_ran_fun(ag, &r, cbs, NULL);

  free_byte_array(r.def);
}

void sm_rrc_stats_trigger_message(struct e2ap_agent_s* ag, uint16_t rnti, rrc_stats_fill_ind_msg_cb cb, void* ctxt)
{
  assert(cb);
  if (g_n_subs == 0)
    return; /* no subscriptions -> nothing to do */

  byte_array_t ind_hdr = rrc_stats_encode_indication_header(rnti);
  pthread_mutex_lock(&g_subs_mtx);
  for (size_t i = 0; i < g_n_subs; ++i) {
    byte_array_t ind_msg = rrc_stats_encode_indication_message(cb, &g_subs[i].style, ctxt);
    const ric_indication_t ind = {
      .ric_id = g_subs[i].sr->ric_id,
      .action_id = g_subs[i].sr->action[0].id,
      .type = RIC_IND_REPORT,
      .hdr = ind_hdr,
      .msg = ind_msg,
    };
    e2ap_send_indication_agent(ag, 0, &ind);
  }
  pthread_mutex_unlock(&g_subs_mtx);
}
