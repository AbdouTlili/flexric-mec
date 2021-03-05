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

#ifndef E2AP_ENDPOINT_RIC_H
#define E2AP_ENDPOINT_RIC_H

#include "e2ap_encode.h"
#include "e2ap_ep.h"
#include "type_defs.h"

#ifdef PROFILE_THROUGHPUT
#include <time.h>
// for struct timespec and clock_gettime()

typedef struct ep_stats {
  uint32_t num_pkts;
  uint64_t num_bytes;
} ep_stats_t;
#endif

typedef struct
{
  e2ap_ep_t base;

#ifdef PROFILE_THROUGHPUT
  ep_stats_t tx;
  ep_stats_t tx_last;
  struct timespec tx_tlast;

  ep_stats_t rx;
  ep_stats_t rx_last;
  struct timespec rx_tlast;
#endif
} e2ap_ep_ric_t;

void e2ap_init_ep_ric(e2ap_ep_ric_t* ep, const char* addr, int port);

e2ap_msg_t e2ap_recv_msg_ric(e2ap_ep_ric_t* ep, e2ap_enc_t* enc);

void e2ap_send_bytes_ric(e2ap_ep_ric_t* ep, byte_array_t ba);

void e2ap_free_ep_ric(e2ap_ep_ric_t* ep);

#endif

