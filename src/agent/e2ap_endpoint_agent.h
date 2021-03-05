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

#ifndef E2AP_ENDPOINT_AGENT
#define E2AP_ENDPOINT_AGENT

#include "type_defs.h"
#include "e2ap_ep.h"
#include "hash_table.h"

typedef struct e2ap_ep_ag
{
  e2ap_ep_t base;
  htable_t assoc_ues;
} e2ap_ep_ag_t;



void e2ap_init_ep_agent(e2ap_ep_ag_t* ep, const char* addr, int port);

struct e2ap_enc;
e2ap_msg_t e2ap_recv_msg_agent(e2ap_ep_ag_t* ep, struct e2ap_enc* enc);

void e2ap_send_bytes_agent(e2ap_ep_ag_t* ep, byte_array_t ba);

void e2ap_free_ep_agent(e2ap_ep_ag_t* ep);

#endif

