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

#include "e2ap_endpoint_ric.h"
#include "e2ap_msg_dec_generic.h" 
#include "e2ap_recv_msg.h"

#include <stdio.h>

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

  printf("Init server %s:%d\n", addr, port);
  *(int*)(&ep->base.fd) = init_sctp_conn_server(addr, port);
  *(int*)(&ep->base.port) = port;
  strncpy((char*)(&ep->base.addr), addr, 16);
  printf("Init server fd = %d\n", ep->base.fd);

#ifdef PROFILE_THROUGHPUT
  clock_gettime(CLOCK_MONOTONIC_RAW, &ep->tx_tlast);
  clock_gettime(CLOCK_MONOTONIC_RAW, &ep->rx_tlast);
#endif
}

e2ap_msg_t e2ap_recv_msg_ric(e2ap_ep_ric_t* ep, e2ap_enc_t* enc)
{
  assert(ep != NULL);
  assert(enc != NULL);

  //printf("Message received in the RIC \n");
  BYTE_ARRAY_STACK(ba, 65536);
  e2ap_recv_bytes(&ep->base, &ba);

#ifdef PROFILE_THROUGHPUT
  ep->rx.num_pkts += 1;
  ep->rx.num_bytes += ba.len;
  static int count = 0;
  count++;
  if (count > 1000) {
    count = 0;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    struct timespec *start = &ep->rx_tlast;
    const uint64_t delta_us = (now.tv_sec - start->tv_sec) * 1000000 + (now.tv_nsec - start->tv_nsec) / 1000;
    if (delta_us > 10*1000*1000) {
      // every 10s (or more)
      const uint32_t diff_pkts = ep->rx.num_pkts - ep->rx_last.num_pkts;
      const uint64_t diff_bytes = ep->rx.num_bytes - ep->rx_last.num_bytes;
      ep->rx_last.num_pkts = ep->rx.num_pkts;
      ep->rx_last.num_bytes = ep->rx.num_bytes;
      const float Mbps = (float) (diff_bytes * 8) / delta_us;
      printf("ep %d rx pkts %7d bytes %10ld diff pkts %6d bytes %8ld delta_us %ld avg thr %.3f Mbps\n",
             ep->base.fd, ep->rx.num_pkts, ep->rx.num_bytes,
             diff_pkts, diff_bytes, delta_us, Mbps);
      memcpy(&ep->rx_last, &ep->rx, sizeof(ep->rx));
      memcpy(&ep->rx_tlast, &now, sizeof(now));
    }
  }
#endif

  e2ap_msg_t msg = e2ap_msg_dec(&enc->type, ba);
  return msg;
}

void e2ap_send_bytes_ric(e2ap_ep_ric_t* ep, byte_array_t ba)
{
  assert(ba.buf && ba.len > 0);
  assert(ep != NULL);
  e2ap_send_bytes(&ep->base, ba);
#ifdef PROFILE_THROUGHPUT
  ep->tx.num_pkts += 1;
  ep->tx.num_bytes += ba.len;
  static int count = 0;
  count++;
  if (count > 1000) {
    count = 0;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    struct timespec *start = &ep->tx_tlast;
    const uint64_t delta_us = (now.tv_sec - start->tv_sec) * 1000000 + (now.tv_nsec - start->tv_nsec) / 1000;
    if (delta_us > 10*1000*1000) {
      // every 10s (or more)
      const uint32_t diff_pkts = ep->tx.num_pkts - ep->tx_last.num_pkts;
      const uint64_t diff_bytes = ep->tx.num_bytes - ep->tx_last.num_bytes;
      ep->tx_last.num_pkts = ep->tx.num_pkts;
      ep->tx_last.num_bytes = ep->tx.num_bytes;
      const float Mbps = (float) (diff_bytes * 8) / delta_us;
      printf("ep %d tx pkts %7d bytes %10ld diff pkts %6d bytes %8ld delta_us %ld avg thr %.3f Mbps\n",
             ep->base.fd, ep->tx.num_pkts, ep->tx.num_bytes,
             diff_pkts, diff_bytes, delta_us, Mbps);
      memcpy(&ep->tx_last, &ep->tx, sizeof(ep->tx));
      memcpy(&ep->tx_tlast, &now, sizeof(now));
    }
  }
#endif
  //printf("Server data send through sctp = %ld\n", ba.len);
}

void e2ap_free_ep_ric(e2ap_ep_ric_t* ep)
{
  assert(ep != NULL);
  assert(0!=0 &&"Not implemented");
}

