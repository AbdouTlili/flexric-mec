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

#include "relay_app_rf.h"

static
void* agent_thread(void* data)
{
  e2ap_agent_t* ag = (e2ap_agent_t*) data;
  e2ap_start_agent(ag);
  return NULL;
}

typedef struct relay_sub_s {
  subscription_t* sub;
  relay_t* relay;
} relay_sub_t;

static
void relay_handle_subscription_response(const ric_subscription_response_t *r, void *data)
{
  assert(r);
  assert(data);
  relay_sub_t* relay_sub = (relay_sub_t*) data;
  relay_t* relay = relay_sub->relay;
  subscription_t* sub = relay_sub->sub;

  /* forward to northbound controller */
  const ric_gen_id_t* ric_id = &r->ric_id;
  printf("[RELAY] received subscription response %d/%d/%d, forward to server\n",
         ric_id->ric_req_id, ric_id->ric_inst_id, ric_id->ran_func_id);
  assert(relay->ag);
  e2ap_send_subscription_response(relay->ag, sub->ep_id, r);
}

static
void relay_handle_subscription_failure(const ric_subscription_failure_t* r, void *data)
{
  assert(r);
  assert(data);
  relay_sub_t* relay_sub = (relay_sub_t*) data;
  relay_t* relay = relay_sub->relay;
  subscription_t* sub = relay_sub->sub;

  /* forward to northbound controller */
  const ric_gen_id_t* ric_id = &r->ric_id;
  printf("[RELAY] received subscription failure %d/%d/%d, forward to server and drop subscription\n",
         ric_id->ric_req_id, ric_id->ric_inst_id, ric_id->ran_func_id);
  assert(relay->ag);
  e2ap_send_subscription_failure(relay->ag, sub->ep_id, r);
  e2ap_delete_subscription(relay->ag, sub->ep_id, *ric_id);
}

static
void relay_handle_indication(const ric_indication_t *i, void* data)
{
  assert(i);
  assert(data);
  relay_sub_t* relay_sub = (relay_sub_t*) data;
  relay_t* relay = relay_sub->relay;
  subscription_t* sub = relay_sub->sub;

  /* forward to northbound controller */
  const ric_gen_id_t* ric_id = &i->ric_id;
  printf("[RELAY] received indication %d/%d/%d, forward to server\n",
         ric_id->ric_req_id, ric_id->ric_inst_id, ric_id->ran_func_id);
  assert(relay->ag);
  e2ap_send_indication_agent(relay->ag, sub->ep_id, i);
}

static
bool relay_handle_subscription_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->ep_id == 0);
  assert(rfdata);
  relay_t* relay = (relay_t*) rfdata;

  /* forward to southbound agent */
  const ric_subscription_request_t* sr = sub->req;
  const ric_gen_id_t* ric_id = &sr->ric_id;
  printf("[RELAY] received subscription request %d/%d/%d, forward to agent\n",
         ric_id->ric_req_id, ric_id->ric_inst_id, ric_id->ran_func_id);
  relay_sub_t* relay_sub = malloc(sizeof(*relay_sub));
  relay_sub->sub = sub;
  relay_sub->relay = relay;
  cb_sub_t cb = {
    .sub_resp = relay_handle_subscription_response,
    .sub_fail = relay_handle_subscription_failure,
    .ind = relay_handle_indication
  };
  int assoc_id = relay->ran->entities.nosplit.full->assoc_id;
  e2ap_subscribe(relay->local_ric, assoc_id, sub->req, cb, relay_sub);
  return true;
}

static
void relay_handle_subscription_delete_response(const ric_subscription_delete_response_t* dr, void *data)
{
  assert(dr);
  assert(data);
  relay_sub_t* relay_sub = (relay_sub_t*) data;
  relay_t* relay = relay_sub->relay;
  subscription_t* sub = relay_sub->sub;

  /* forward to northbound controller */
  const ric_gen_id_t* ric_id = &dr->ric_id;
  printf("[RELAY] received subscription delete response %d/%d/%d, forward to server\n",
         ric_id->ric_req_id, ric_id->ric_inst_id, ric_id->ran_func_id);
  assert(relay->ag);
  e2ap_send_subscription_delete_response(relay->ag, sub->ep_id, dr);
  e2ap_delete_subscription(relay->ag, sub->ep_id, *ric_id);
  free(relay_sub);
}

static
void relay_handle_subscription_delete_failure(const ric_subscription_delete_failure_t* df, void *data)
{
  assert(df);
  assert(data);
  relay_sub_t* relay_sub = (relay_sub_t*) data;
  relay_t* relay = relay_sub->relay;
  subscription_t* sub = relay_sub->sub;

  /* forward to northbound controller */
  const ric_gen_id_t* ric_id = &df->ric_id;
  printf("[RELAY] received subscription delete failure %d/%d/%d, forward to server\n",
         ric_id->ric_req_id, ric_id->ric_inst_id, ric_id->ran_func_id);
  assert(relay->ag);
  e2ap_send_subscription_delete_failure(relay->ag, sub->ep_id, df);
}

static
bool relay_handle_subscription_delete_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub);
  assert(rfdata);
  relay_t* relay = (relay_t*) rfdata;

  /* forward to southbound agent */
  const ric_subscription_request_t* sr = sub->req;
  const ric_gen_id_t* ric_id = &sr->ric_id;
  printf("[RELAY] received subscription delete request %d/%d/%d, forward to agent\n",
         ric_id->ric_req_id, ric_id->ric_inst_id, ric_id->ran_func_id);
  cb_sub_del_t cb = {
    .sub_del_resp = relay_handle_subscription_delete_response,
    .sub_del_fail = relay_handle_subscription_delete_failure
  };
  int assoc_id = relay->ran->entities.nosplit.full->assoc_id;
  const ric_subscription_delete_request_t sd = { .ric_id = *ric_id };
  e2ap_unsubscribe(relay->local_ric, assoc_id, &sd, cb);
  /* do NOT free corresponding subscription yet: we await the response of the
   * agent to decide whether we need to dispose this subscription */
  return false;
}

static
void relay_handle_control_acknowledge(const ric_control_acknowledge_t* a, void* data)
{
  assert(a);
  assert(data);
  relay_t* relay = (relay_t*) data;

  /* forward to northbound controller */
  const ric_gen_id_t* ric_id = &a->ric_id;
  printf("[RELAY] received control acknowledge %d/%d/%d, forward to server\n",
         ric_id->ric_req_id, ric_id->ric_inst_id, ric_id->ran_func_id);
  assert(relay->ag);
  e2ap_send_control_acknowledge(relay->ag, 0, a);
}

void relay_handle_control_failure(const ric_control_failure_t* f, void* data)
{
  assert(f);
  assert(data);
  relay_t* relay = (relay_t*) data;

  /* forward to northbound controller */
  const ric_gen_id_t* ric_id = &f->ric_id;
  printf("[RELAY] received control failure %d/%d/%d, forward to server\n",
         ric_id->ric_req_id, ric_id->ric_inst_id, ric_id->ran_func_id);
  assert(relay->ag);
  e2ap_send_control_failure(relay->ag, 0, f);
}

static
void relay_handle_control_request(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_request_t* cr, void* rfdata)
{
  assert(ag);
  assert(ep_id == 0);
  assert(cr);
  assert(rfdata);
  relay_t* relay = (relay_t*) rfdata;

  /* forward to southbound agent */
  const ric_gen_id_t* ric_id = &cr->ric_id;
  printf("[RELAY] received control request %d/%d/%d, forward to agent\n",
         ric_id->ric_req_id, ric_id->ric_inst_id, ric_id->ran_func_id);
  cb_ctrl_t cb = {
    .ctrl_ack = relay_handle_control_acknowledge,
    .ctrl_fail = relay_handle_control_failure
  };
  int assoc_id = relay->ran->entities.nosplit.full->assoc_id;
  e2ap_control_req(relay->local_ric, assoc_id, cr, cb, relay);
}

static
void relay_app_ran_connect(const ran_t* ran, void *data)
{
  assert(ran && data);
  relay_t* relay = (relay_t*) data;

  assert(!relay->ag && !relay->ran && "cannot handle more than one agent connection");

  /* ToDO: merge multiple RANs and forward on a single connection */
  assert(ran->entities.split == SPLIT_NONE && ran->entities.nosplit.full);
  const e2_setup_request_t* sr = &ran->entities.nosplit.full->e2_setup_request;
  const int assoc_id = ran->entities.nosplit.full->assoc_id;
  relay->ran = ran;

  /* a new agent: we open a new connection to the controller above and forward
   * the e2_setup_request */
  relay->ag = e2ap_init_agent(relay->remote_addr, relay->remote_port, sr->id);
  //relay->ag = e2ap_create_instance_agent();
  //set_fd_non_blocking(relay->ag->efd);
  //ep_id_t ep_id = add_endpoint(relay->ag, relay->remote_addr, relay->remote_port, sr);
  
  /* for every RF in SR: add RF in agent for handling incoming messages
   * and forwarding them down to sub-agent. */
  service_model_cb_t cbs = {
    .handle_subscription_request = relay_handle_subscription_request,
    .handle_subscription_delete_request = relay_handle_subscription_delete_request,
    .handle_control_request = relay_handle_control_request,
  };
  for (size_t i = 0; i < sr->len_rf; ++i) {
    e2ap_register_ran_fun(relay->ag, &sr->ran_func_item[i], cbs, relay);
  }

  printf("[RELAY] new RAN %ld/assoc id %d forwarding to server %s:%d/agent %p\n",
         ran->generated_ran_key, assoc_id, relay->remote_addr, relay->remote_port, relay->ag);

  int rc = pthread_create(&relay->ag_thread, NULL, agent_thread, relay->ag);
  assert(rc == 0);
}

void relay_app_init(relay_t* relay, e2ap_ric_t* local_ric, const char* remote_addr, int remote_port)
{
  assert(relay);
  assert(local_ric);
  assert(remote_addr);

  relay->local_ric = local_ric;
  relay->remote_addr = strdup(remote_addr);
  relay->remote_port = remote_port;
  relay->ag = NULL;
  relay->ag_thread = 0;
  relay->ran = NULL;

  ed_t ed = subscribe_ran_connect(&local_ric->events, relay_app_ran_connect, relay);
  (void) ed;
  printf("[RELAY] init relay done: forward to controller at %s:%d\n", remote_addr, remote_port);
}

void relay_app_free(relay_t* relay)
{
  assert(relay);
  /* ToDO: delete remaining subscriptions? */
  if (relay->ag) {
    e2ap_free_agent(relay->ag);
    relay->ag = NULL;
  }
  free(relay->remote_addr);
}
