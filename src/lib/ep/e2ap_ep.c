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

#include "e2ap_ep.h"
#include "../../util/alg_ds/ds/lock_guard/lock_guard.h"

#include <pthread.h>

void e2ap_ep_init(e2ap_ep_t* ep)
{
  int rc = pthread_mutex_init(&ep->mtx, NULL);
  assert(rc == 0);
}

void e2ap_ep_free(e2ap_ep_t* ep)
{
  assert(ep != NULL);
  int rc = pthread_mutex_destroy(&ep->mtx);
  assert(rc == 0);
}

void e2ap_send_sctp_msg(const e2ap_ep_t* ep, sctp_msg_t* msg)
{
  assert(ep != NULL);
  assert(msg->ba.buf && msg->ba.len > 0);

  struct sockaddr_in const* addr = &msg->info.addr; 
  struct sctp_sndrcvinfo const* sri = &msg->info.sri;
  byte_array_t const ba = msg->ba;

  lock_guard(&((e2ap_ep_t*)ep)->mtx);

  const int rc = sctp_sendmsg(ep->fd, (void*)ba.buf, ba.len, (struct sockaddr *)addr, sizeof(*addr), sri->sinfo_ppid, sri->sinfo_flags, sri->sinfo_stream, 0, 0) ;
  assert(rc != 0);
}

sctp_msg_t e2ap_recv_sctp_msg(e2ap_ep_t* ep)
{
  assert(ep != NULL);

  sctp_msg_t from = {0}; 
  from.ba.len = 2048;
  from.ba.buf = malloc(2048);
  assert(from.ba.buf != NULL && "Memory exhausted");

  socklen_t len = sizeof(from.info.addr);
  int msg_flags = 0;

  lock_guard(&((e2ap_ep_t*)ep)->mtx);
  int const rc = sctp_recvmsg(ep->fd, from.ba.buf, from.ba.len, (struct sockaddr*)&from.info.addr, &len, &from.info.sri, &msg_flags);
  assert(rc > -1 && rc != 0);
  assert((msg_flags == MSG_NOTIFICATION  || msg_flags == MSG_EOR) && "Error, horror"); // Notification or end of record

  from.ba.len = rc; // set actually received number of bytes
  return from;
}

