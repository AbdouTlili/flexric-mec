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

// File to simulate the RIC behaviour

#ifndef E2_SERVER_H
#define E2_SERVER_H

//#include <stdatomic.h>
#include <stdbool.h>

#include "e2ap_msg_handler_ric.h"
#include "e2ap_encode.h"
#include "e2ap_endpoint_ric.h"
#include "ran_mgmt.h"
#include "sub_mgmt.h"
#include "events.h"

typedef struct e2ap_ric_s
{
  bool server_stopped;
  bool stop_token;
  e2ap_ep_ric_t ep;
  e2ap_enc_t enc;
  int efd; // epoll fd
  
 e2ap_handle_msg_fp_ric handle_msg[26]; // note that not all the slots will be occupied

  ran_mgmt_t ran_mgmt;
  sub_mgmt_t sub_mgmt;
  events_t events;

} e2ap_ric_t;

typedef void (*ric_timer_cb_t)(e2ap_ric_t* ric, int fd, void* data);
int e2ap_add_timer_epoll_ms_server(e2ap_ric_t* ric, long initial_ms, long interval_ms, ric_timer_cb_t cb, void* data);
void e2ap_remove_timer_epoll_server(int tfd);

void e2ap_init_server(e2ap_ric_t* ag, const char* addr, int port);

void e2ap_free_server(e2ap_ric_t* );

void e2ap_start_server(e2ap_ric_t* ric);

void e2ap_stop_server(e2ap_ric_t* ric);

void e2ap_subscribe(e2ap_ric_t* ric, int assoc_id, const ric_subscription_request_t* req, cb_sub_t cb, void* data);

void e2ap_unsubscribe(e2ap_ric_t* ric, int assoc_id, const ric_subscription_delete_request_t* del, cb_sub_del_t cb);

void e2ap_control_req(e2ap_ric_t* ric, int assoc_id, const ric_control_request_t* req, cb_ctrl_t cb, void* data);

#endif

