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


#include "endpoint_ric.h"
#include <arpa/inet.h>   // for inet_pton
#include <assert.h>      // for assert
#include <errno.h>       // for errno
//#include <linux/sctp.h>  // for sctp_event_subscribe, SCTP_AUTOCLOSE, SCTP_E...
#include <netinet/sctp.h>
#include <netinet/in.h>  // for sockaddr_in, IPPROTO_SCTP, htons, sockaddr_in6
#include <stdio.h>       // for printf, NULL, size_t
#include <stdlib.h>      // for malloc
#include <string.h>      // for memset, strlen, strncpy
#include <strings.h>     // for bzero
#include <sys/socket.h>  // for setsockopt, AF_INET, bind, listen, socket

static
const int SERVER_LISTEN_QUEUE_SIZE = 32;

static
int init_sctp_conn_server(const char* addr, int port)
{
  errno = 0;
  struct sockaddr_in  server4_addr;
  memset(&server4_addr, 0, sizeof(struct sockaddr_in));
  if(inet_pton(AF_INET, addr, &server4_addr.sin_addr) != 1){
    // Error occurred
    struct sockaddr_in6 server6_addr;
    memset(&server6_addr, 0, sizeof(struct sockaddr_in6));
    if(inet_pton(AF_INET6, addr, &server6_addr.sin6_addr) == 1){
      assert(0!=0 && "IPv6 not supported");
    }
    assert(0!=0 && "Incorrect IP address string.");
  }

  server4_addr.sin_family = AF_INET;
  server4_addr.sin_port = htons(port);
  const int server_fd = socket (AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
  assert(server_fd != -1);

  const size_t addr_len = sizeof(server4_addr);
  struct sockaddr* server_addr = (struct sockaddr*)&server4_addr;
  int rc = bind(server_fd, server_addr, addr_len);
  if(rc == -1){
    printf("errno = %d\n", errno);
  }
  assert(rc != -1);

  struct sctp_event_subscribe evnts;
  bzero (&evnts, sizeof (evnts)) ;
  evnts.sctp_data_io_event = 1;
  rc = setsockopt (server_fd, IPPROTO_SCTP, SCTP_EVENTS, &evnts, sizeof (evnts));
  assert(rc != -1);

  const int close_time = 120;
  setsockopt(server_fd, IPPROTO_SCTP, SCTP_AUTOCLOSE, &close_time, sizeof(close_time));

  rc = listen(server_fd, SERVER_LISTEN_QUEUE_SIZE);
  assert(rc != -1);
  assert(errno == 0);
  return server_fd;
}

void e2ap_init_ep_ric(e2ap_ep_ric_t* ep, const char* addr, int port)
{
  assert(ep != NULL);
  assert(addr != NULL);
  assert(strlen(addr) < 16);
  assert(port > 0 && port < 65535);

  e2ap_ep_init(&ep->base); 

  *(int*)(&ep->base.fd) = init_sctp_conn_server(addr, port);
  *(int*)(&ep->base.port) = port;
  strncpy((char*)(&ep->base.addr), addr, 16);
  printf("[NEAR-RIC]: Initializing \n"); //server fd = %d\n", ep->base.fd);
}

byte_array_t e2ap_recv_msg_ric(e2ap_ep_ric_t* ep)
{
  assert(ep != NULL);

  // iff performance needed, pass the argument by reference. Pls, measure before doing it 
  byte_array_t ba = {.buf = malloc(2048) , .len =2048  };
  e2ap_recv_bytes(&ep->base, &ba);
//  printf("Number of bytes read = %lu \n", ba.len);
  return ba;
}

void e2ap_send_bytes_ric(const e2ap_ep_ric_t* ep, byte_array_t ba)
{
  assert(ba.buf && ba.len > 0);
  assert(ep != NULL);
  e2ap_send_bytes(&ep->base, ba);
  //printf("Server data send through sctp = %ld\n", ba.len);
}

void e2ap_free_ep_ric(e2ap_ep_ric_t* ep)
{
  assert(ep != NULL);
  e2ap_ep_free(&ep->base);
}

