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

#include "e2ap_recv_msg.h"
#include "type_defs.h"
#include "e2ap_encode.h"

#include <assert.h>
#include <stdio.h>

void e2ap_recv_bytes(e2ap_ep_t* ep, byte_array_t* ba)
{
  assert(ep != NULL);

  socklen_t len = sizeof(ep->to);
  const int rc = sctp_recvmsg(ep->fd, ba->buf, ba->len, (struct sockaddr*)&ep->to, &len, &ep->sri, &ep->msg_flags);
  assert(rc > -1 && rc < (int)ba->len);
  //printf("Received data: %s from client and number of bytes = %d\n", ba->buf, rc);
  //fflush(stdout);
  ba->len = rc; // set actually received number of bytes
}
