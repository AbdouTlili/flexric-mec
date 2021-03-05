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

void e2ap_send_bytes(const e2ap_ep_t* ep, byte_array_t ba)
{
  assert(ep != NULL);
  assert(ba.buf && ba.len > 0);

  const int rc = sctp_sendmsg(ep->fd, (void*)ba.buf, ba.len, (struct sockaddr *)&ep->to, sizeof(ep->to), ep->sri.sinfo_ppid, ep->sri.sinfo_flags, ep->sri.sinfo_stream, 0, 0) ;
  assert(rc != 0);
 
/*
  const int ret = sctp_sendmsg(ep->fd, (void *)ba.buf, ba.len, (struct sockaddr *)&ep->to, sizeof(ep->to), 0, 0, 0, 0, 0 );
  assert(ret != 0);
  printf("Client data send through sctp = %ld\n", ba.len);
  */
}


