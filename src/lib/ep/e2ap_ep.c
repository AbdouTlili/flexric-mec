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


void e2ap_send_bytes(const e2ap_ep_t* ep, byte_array_t ba)
{
  assert(ep != NULL);
  assert(ba.buf && ba.len > 0);

  int rc_mtx = pthread_mutex_lock(&((e2ap_ep_t*)ep)->mtx);
  assert(rc_mtx == 0);
  const int rc = sctp_sendmsg(ep->fd, (void*)ba.buf, ba.len, (struct sockaddr *)&ep->to, sizeof(ep->to), ep->sri.sinfo_ppid, ep->sri.sinfo_flags, ep->sri.sinfo_stream, 0, 0) ;
  rc_mtx = pthread_mutex_unlock(&((e2ap_ep_t*)ep)->mtx);
  assert(rc_mtx == 0);
  assert(rc != 0);
}

void e2ap_recv_bytes(e2ap_ep_t* ep, byte_array_t* ba)
{
  assert(ep != NULL);

  socklen_t len = sizeof(ep->to);

  int rc_mtx = pthread_mutex_lock(&((e2ap_ep_t*)ep)->mtx);
  assert(rc_mtx == 0);
  const int rc = sctp_recvmsg(ep->fd, ba->buf, ba->len, (struct sockaddr*)&ep->to, &len, &ep->sri, &ep->msg_flags);
  rc_mtx = pthread_mutex_unlock(&((e2ap_ep_t*)ep)->mtx);
  assert(rc_mtx == 0);

  assert(rc > -1);
  assert(rc != 0);
  //printf("Received data: %s from client and number of bytes = %d\n", ba->buf, rc);
  //fflush(stdout);
  ba->len = rc; // set actually received number of bytes
}
