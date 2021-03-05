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

#ifndef __FLEXRIC_AGENT_H_
#define __FLEXRIC_AGENT_H_

#include <stdbool.h>

#include "ngran_types.h"
#include "global_consts.h"
#include "type_defs.h"
#include "rb_tree.h"

typedef struct e2ap_agent_s e2ap_agent_t;
typedef int ep_id_t;

typedef struct subscription_s {
  const ric_subscription_request_t* req;
  const ep_id_t ep_id;
  void* data;
} subscription_t;

typedef struct service_model_cb {
  // Return true if subscription is accepted, in which case the agent maintains
  // the subscription_t*. The callback has to trigger the subscription
  // response/failure!
  bool (*handle_subscription_request)(e2ap_agent_t*, subscription_t*, void *rfdata);
  // Return true if subscription delete is accepted, in which case the agent
  // frees the ric_subscription_request_t* structure, otherwise it remains. The
  // callback has to trigger the subscription delete response/failure!
  bool (*handle_subscription_delete_request)(e2ap_agent_t*, subscription_t*, void *rfdata);
  // The callback has to send the control acknowledge/failure message!
  void (*handle_control_request)(e2ap_agent_t*, ep_id_t ep_id, const ric_control_request_t*, void *rfdata);
} service_model_cb_t;


typedef void (*timer_cb_t)(e2ap_agent_t* ag, int fd, void* data);
int e2ap_add_timer_epoll_ms_agent(e2ap_agent_t* ag, long initial_ms, long interval_ms, timer_cb_t cb, void* data);
void e2ap_remove_timer_epoll_agent(int tfd);


e2ap_agent_t* e2ap_init_agent(const char* addr, int port, global_e2_node_id_t ge2nid);
void e2ap_register_ran_fun(e2ap_agent_t* ag, const ran_function_t* r, service_model_cb_t cbs, void *rfdata);
void e2ap_start_agent(e2ap_agent_t* ag);
void e2ap_free_agent(e2ap_agent_t* ag);


void e2ap_send_subscription_response(e2ap_agent_t* ag, ep_id_t ep_id, const ric_subscription_response_t* sr);
void e2ap_send_subscription_failure(e2ap_agent_t* ag, ep_id_t ep_id, const ric_subscription_failure_t* sf);

void e2ap_send_indication_agent(e2ap_agent_t* ag, ep_id_t ep_id, const ric_indication_t* indication);

void e2ap_send_subscription_delete_response(e2ap_agent_t* ag, ep_id_t ep_id, const ric_subscription_delete_response_t* sdr);
void e2ap_send_subscription_delete_failure(e2ap_agent_t* ag, ep_id_t ep_id, const ric_subscription_delete_failure_t* sdf);

void e2ap_send_control_acknowledge(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_acknowledge_t* ca);
void e2ap_send_control_failure(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_failure_t* cf);

subscription_t* e2ap_find_subscription(e2ap_agent_t* ag, ep_id_t ep_id, ric_gen_id_t id);
void e2ap_delete_subscription(e2ap_agent_t* ag, ep_id_t ep_id, ric_gen_id_t id);

#endif /* __FLEXRIC_AGENT_H_ */
