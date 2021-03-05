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

#include "e2ap_endpoint_agent.h"

#include "e2ap_ep.h"
#include "e2ap_msg_dec_generic.h" 
#include "e2ap_recv_msg.h"
#include "e2ap_encode.h"

#include <stdio.h>

static
void init_sctp_conn_client(e2ap_ep_ag_t* ep, const char* addr, int port)
{
  int sock_fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
  assert(sock_fd != -1);

  struct sockaddr_in servaddr; 
  bzero(&servaddr, sizeof (servaddr) ) ;
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
  servaddr.sin_port = htons (port);
  int rc = inet_pton(AF_INET, addr, &servaddr.sin_addr);
  assert(rc == 1);

  struct sctp_event_subscribe evnts; 
  bzero(&evnts, sizeof (evnts)) ;
  evnts.sctp_data_io_event = 1 ;
  setsockopt(sock_fd, IPPROTO_SCTP, SCTP_EVENTS, &evnts, sizeof (evnts));

  const int no_delay = 1;
  setsockopt(sock_fd, IPPROTO_SCTP, SCTP_NODELAY, &no_delay, sizeof(no_delay));

  ep->base.to = servaddr;
  *(int*)(&ep->base.port) = port; 
  *(int*)(&ep->base.fd) = sock_fd;
  strncpy((char*)(&ep->base.addr), addr, 16);
}

void e2ap_init_ep_agent(e2ap_ep_ag_t* ep, const char* addr, int port)
{
  assert(ep != NULL);
  assert(addr != NULL);
  assert(strlen(addr) < 16);
  assert(port > 0 && port < 65535);
  init_sctp_conn_client(ep, addr, port);
}

e2ap_msg_t e2ap_recv_msg_agent(e2ap_ep_ag_t* ep, e2ap_enc_t* enc)
{
  assert(ep != NULL);
  assert(enc != NULL);

  BYTE_ARRAY_STACK(ba, 11000);
  e2ap_recv_bytes(&ep->base, &ba);
  e2ap_msg_t msg = e2ap_msg_dec(&enc->type, ba);
  //printf("Message received in the agent\n");
  return msg;

/*
  BYTE_ARRAY_STACK(recv, 2048);
  socklen_t len = sizeof(ep->to);
  int rc = -1;
  for(;;){
    rc = sctp_recvmsg(ep->fd, recv.buf, recv.len, (struct sockaddr *) &ep->to, &len, &ep->sri, &ep->msg_flags) ;
    if(rc > -1) break;
    printf("Error = %s\n", strerror(errno));
    fflush(stdout);
    assert(errno == EAGAIN || errno == EWOULDBLOCK);
    usleep(1000);
  } 
  assert(rc > -1);
  printf("Received data: %s from server and number of bytes = %d\n", recv.buf, rc);
  fflush(stdout);
  recv.len = rc; // set actually received number of bytes
  e2ap_msg_t msg =  e2ap_msg_dec(&enc->type, recv);
  return msg;
  */

}

void e2ap_send_bytes_agent(e2ap_ep_ag_t* ep, byte_array_t ba)
{
  assert(ep != NULL);
  assert(ba.buf && ba.len > 0);
  e2ap_send_bytes(&ep->base, ba);
  //printf("Client data send through sctp = %ld\n", ba.len);
  //fflush(stdout);

/*
  const int ret = sctp_sendmsg(ep->fd, (void *)ba.buf, ba.len, (struct sockaddr *)&ep->to, sizeof(ep->to), 0, 0, 0, 0, 0 );
  assert(ret != 0);
  printf("Client data send through sctp = %ld\n", ba.len);
  */
}

void e2ap_free_ep_agent(e2ap_ep_ag_t* ep)
{
  assert(ep != NULL);
  assert(0!=0 && "Not implememented!");
}
