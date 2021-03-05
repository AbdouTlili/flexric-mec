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

#ifndef E2_AGENT_H
#define E2_AGENT_H

#define ASN1

#include "rb_tree.h"

#include "ngran_types.h"
#include "global_consts.h"
#include "type_defs.h"
#include "flexric_agent.h"

#include "e2ap_endpoint_agent.h"
#include "e2ap_encode.h"

#include <stdbool.h>
//#define _GNU_SOURCE

typedef struct ran_function_container {
  ran_function_t* r;
  void *data;
  service_model_cb_t cbs;
  rb_tree_t sub_reqs;
  //rb_tree timers;
} ran_function_container_t;

typedef struct ran_func_s
{
  uint64_t key;
  ran_function_container_t* rf_container;
} pair_ran_fun_t;

pair_ran_fun_t* init_pair_fun_reg(uint64_t key, ran_function_t* r, service_model_cb_t cbs, void* rfdata);
void free_fun_reg(pair_ran_fun_t*);

typedef struct pair_sub_data {
  uint64_t key;
  subscription_t* sub;
} pair_sub_data_t;

pair_sub_data_t* init_pair_sub_data(uint64_t key, subscription_t* sub);
void free_pair_sub_data(pair_sub_data_t*);


struct e2ap_agent_s;
typedef void (*e2ap_handle_msg_fp_agent)(struct e2ap_agent_s*, ep_id_t ep_id, const e2ap_msg_t* msg);

typedef struct e2ap_agent_s 
{
  e2ap_enc_t enc;
  e2ap_ep_ag_t* ep; // multiple controllers, dynamically allocated
  int num_ep;
  int cap_ep;

  rb_tree_t fun_registered;

  e2ap_handle_msg_fp_agent handle_msg[26]; // note that not all the slots will be occupied

  global_e2_node_id_t global_e2_node_id;

  int efd; 
  bool stop_token;
  bool agent_stopped;

} e2ap_agent_t;

e2ap_agent_t* e2ap_create_instance_agent(void);
void e2ap_event_loop_agent(e2ap_agent_t* ag);

void e2ap_register_sub_req_agent(ran_function_container_t* rfc, ep_id_t ep_id, subscription_t* sub);
subscription_t* e2ap_get_sub_req_agent(ran_function_container_t* rfc, ep_id_t ep_id, ric_gen_id_t ric_id);
void e2ap_delete_sub_req_agent(ran_function_container_t* rfc, ep_id_t ep_id, ric_gen_id_t ric_id);

ran_function_container_t* e2ap_get_ran_fun_agent(e2ap_agent_t* ag, uint16_t ran_func_id);

ric_subscription_request_t* deep_copy_sub_req(const ric_subscription_request_t* sr);

void free_subscription_request(ric_subscription_request_t* sr);

void add_fd_epoll(int efd, int fd);
void set_fd_non_blocking(int sfd);

#endif

